/*
 * xor_decrypt.c
 *
 * Recursively XOR-decrypt files under a target directory using a key read from a file.
 * Supports key file containing either raw bytes or hex-encoded ASCII (e.g. "4f2a...").
 *
 * Usage:
 *   xor_decrypt --key /opt/.fixutil/backup.txt --target /usr/local/apache2/htdocs [--dry-run]
 *
 * Compile:
 *   gcc -O2 -Wall -o xor_decrypt xor_decrypt.c -std=c11 -D_XOPEN_SOURCE=700
 */

#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <utime.h>
#include <sys/types.h>
#include <libgen.h>
#include <limits.h>

#define BUF_SIZE 65536
static unsigned char *g_key = NULL;
static size_t g_key_len = 0;
static bool g_dry_run = false;
static char g_target_realpath[PATH_MAX] = {0};

/* forward */
bool load_key(const char *keypath);
static int process_one_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

/* utility: is hex digit */
static inline int is_hex_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Parse hex string (ASCII) into bytes. Returns malloc'd buffer and length via out_len, or NULL on error. */
unsigned char *parse_hex_bytes(const char *s, size_t *out_len) {
    size_t slen = strlen(s);
    // strip whitespace/newline
    char *buf = malloc(slen + 1);
    if (!buf) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < slen; ++i) {
        if (!isspace((unsigned char)s[i])) buf[j++] = s[i];
    }
    buf[j] = '\0';
    if (j % 2 != 0) { free(buf); return NULL; }
    size_t b_len = j / 2;
    unsigned char *out = malloc(b_len);
    if (!out) { free(buf); return NULL; }
    for (size_t i = 0; i < b_len; ++i) {
        char hi = buf[2*i];
        char lo = buf[2*i+1];
        int hv = (hi >= '0' && hi <= '9') ? hi - '0' :
                 (hi >= 'a' && hi <= 'f') ? 10 + hi - 'a' :
                 (hi >= 'A' && hi <= 'F') ? 10 + hi - 'A' : -1;
        int lv = (lo >= '0' && lo <= '9') ? lo - '0' :
                 (lo >= 'a' && lo <= 'f') ? 10 + lo - 'a' :
                 (lo >= 'A' && lo <= 'F') ? 10 + lo - 'A' : -1;
        if (hv < 0 || lv < 0) { free(buf); free(out); return NULL; }
        out[i] = (unsigned char)((hv << 4) | lv);
    }
    free(buf);
    *out_len = b_len;
    return out;
}

/* Load key: detect hex or raw */
bool load_key(const char *keypath) {
    FILE *f = fopen(keypath, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open key file '%s': %s\n", keypath, strerror(errno));
        return false;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return false; }
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return false; }
    if (fread(buf, 1, sz, f) != (size_t)sz) {
        // still possible small read; continue
    }
    buf[sz] = '\0';
    fclose(f);

    // Determine if content is hex (ignoring whitespace). If all non-whitespace chars are hex digits and length even -> parse hex.
    size_t nonspace = 0;
    size_t hexchars = 0;
    for (long i = 0; i < sz; ++i) {
        unsigned char c = buf[i];
        if (!isspace(c)) {
            nonspace++;
            if (is_hex_char(c)) hexchars++;
        }
    }
    if (nonspace > 0 && nonspace == hexchars && (nonspace % 2 == 0)) {
        size_t parsed_len = 0;
        unsigned char *parsed = parse_hex_bytes(buf, &parsed_len);
        if (parsed) {
            g_key = parsed;
            g_key_len = parsed_len;
            free(buf);
            return g_key_len > 0;
        }
    }

    // Otherwise treat as raw bytes (trim trailing newline)
    // Remove trailing newline(s)
    while (sz > 0 && (buf[sz-1] == '\n' || buf[sz-1] == '\r')) sz--;
    if (sz == 0) { free(buf); fprintf(stderr, "Key file appears empty after trimming newlines\n"); return false; }
    g_key = malloc(sz);
    if (!g_key) { free(buf); return false; }
    memcpy(g_key, buf, sz);
    g_key_len = sz;
    free(buf);
    return true;
}

/* XOR-process one file: read in chunks, write to temp, then atomically rename over original.
 * Preserves file mode (and owner if possible) and timestamps.
 */
int xor_process_file_path(const char *path, const struct stat *st) {
    if (!S_ISREG(st->st_mode)) return 0;
    if (st->st_size == 0) {
        // nothing to do
        return 0;
    }

    if (g_dry_run) {
        printf("[DRY RUN] Would process: %s (size=%lld)\n", path, (long long)st->st_size);
        return 0;
    }

    // Open input
    int infd = open(path, O_RDONLY);
    if (infd < 0) {
        fprintf(stderr, "Failed to open %s for reading: %s\n", path, strerror(errno));
        return -1;
    }

    // Prepare temp file in same directory for atomic replace
    char dirbuf[PATH_MAX];
    strncpy(dirbuf, path, PATH_MAX-1);
    dirbuf[PATH_MAX-1] = '\0';
    char *dir = dirname(dirbuf);
    char tmp_template[PATH_MAX];
    snprintf(tmp_template, PATH_MAX, "%s/.xor_tmp_XXXXXX", dir);
    int tmpfd = mkstemp(tmp_template);
    if (tmpfd < 0) {
        fprintf(stderr, "Failed to create temp file in %s: %s\n", dir, strerror(errno));
        close(infd);
        return -1;
    }

    // Set tmp file mode to the original file's permissions
    if (fchmod(tmpfd, st->st_mode & 07777) != 0) {
        // not fatal
    }

    unsigned char *buf = malloc(BUF_SIZE);
    if (!buf) {
        fprintf(stderr, "Out of memory\n");
        close(infd);
        close(tmpfd);
        unlink(tmp_template);
        return -1;
    }

    ssize_t r;
    off_t total = 0;
    size_t keypos = 0;
    while ((r = read(infd, buf, BUF_SIZE)) > 0) {
        for (ssize_t i = 0; i < r; ++i) {
            buf[i] ^= g_key[keypos];
            keypos++;
            if (keypos >= g_key_len) keypos = 0;
        }
        ssize_t woff = 0;
        while (woff < r) {
            ssize_t w = write(tmpfd, buf + woff, r - woff);
            if (w <= 0) {
                fprintf(stderr, "Write error to temp file %s: %s\n", tmp_template, strerror(errno));
                free(buf);
                close(infd);
                close(tmpfd);
                unlink(tmp_template);
                return -1;
            }
            woff += w;
        }
        total += r;
    }
    free(buf);
    if (r < 0) {
        fprintf(stderr, "Read error from %s: %s\n", path, strerror(errno));
        close(infd);
        close(tmpfd);
        unlink(tmp_template);
        return -1;
    }

    // flush and sync
    fsync(tmpfd);
    close(infd);

    // preserve times: use utimensat on target after replace
    struct timespec times[2];
#ifdef __linux__
    times[0] = st->st_atim;
    times[1] = st->st_mtim;
#else
    times[0].tv_sec = st->st_atime;
    times[0].tv_nsec = 0;
    times[1].tv_sec = st->st_mtime;
    times[1].tv_nsec = 0;
#endif

    // close tmpfd before rename on some filesystems? keep it closed to be safe
    close(tmpfd);

    // Atomically replace
    if (rename(tmp_template, path) != 0) {
        fprintf(stderr, "Failed to replace %s with temp file %s: %s\n", path, tmp_template, strerror(errno));
        unlink(tmp_template);
        return -1;
    }

    // restore timestamps
    if (utimensat(AT_FDCWD, path, times, 0) != 0) {
        // not fatal
    }

    printf("Processed: %s (bytes=%lld)\n", path, (long long)total);
    return 0;
}

/* nftw callback wrapper */
static int process_one_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    // Only regular files
    if (typeflag == FTW_F) {
        return xor_process_file_path(fpath, sb);
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *keypath = NULL;
    const char *target = NULL;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s --key /path/to/keyfile --target /path/to/dir [--dry-run]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--key") == 0 && i+1 < argc) { keypath = argv[++i]; continue; }
        if (strcmp(argv[i], "--target") == 0 && i+1 < argc) { target = argv[++i]; continue; }
        if (strcmp(argv[i], "--dry-run") == 0) { g_dry_run = true; continue; }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s --key /path/to/keyfile --target /path/to/dir [--dry-run]\n", argv[0]);
            return 0;
        }
    }

    if (!keypath || !target) {
        fprintf(stderr, "Missing --key or --target\n");
        return 1;
    }

    if (!load_key(keypath)) {
        fprintf(stderr, "Failed to load key from %s\n", keypath);
        return 1;
    }
    if (g_key_len == 0) {
        fprintf(stderr, "Key length is zero\n");
        return 1;
    }

    // Resolve target realpath for safety
    if (!realpath(target, g_target_realpath)) {
        fprintf(stderr, "Failed to resolve target path '%s': %s\n", target, strerror(errno));
        // continue with target string anyway
        strncpy(g_target_realpath, target, PATH_MAX-1);
    }

    printf("Key loaded (%zu bytes). Target: %s. Dry-run: %s\n", g_key_len, g_target_realpath, g_dry_run ? "YES" : "NO");
    printf("Starting recursive traversal...\n");

    // nftw depth-first, max FD 20
    int flags = FTW_PHYS; // do not follow symlinks
    if (nftw(g_target_realpath, process_one_file, 20, flags) != 0) {
        fprintf(stderr, "nftw reported an error\n");
        // proceed to free key and exit non-zero
        free(g_key);
        return 1;
    }

    free(g_key);
    printf("Done.\n");
    return 0;
}
