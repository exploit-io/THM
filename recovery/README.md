# ♻️ Recovery

* Solving [Recovery](https://tryhackme.com/room/recovery)

## 🔥 Introduction

💀 **Ransomware** is no longer a distant, abstract threat—it’s a personal disaster waiting to happen, as Alex from Recoverysoft learned the hard way. 💻 What began as a routine email from a trusted colleague quickly spiraled into chaos: a simple click, a harmless-looking “fix” binary, and within hours, his company’s Ubuntu web server was locked down by a mocking message ⚠️ `YOU DIDN’T SAY THE MAGIC WORD!` With files encrypted 🔒, SSH blocked 🚫, and backdoors lurking in system libraries 🧨, Alex’s nightmare encapsulates the deceptive power and devastating impact of modern ransomware. His story serves as a stark reminder ⚔️ that in today’s digital world, trust can be weaponized, and one careless command can bring an entire system to its knees. 🕹️


Now Alex, Asks US to recover ♻️ accessing his server and 🗂️ files with following credentials:
```
Username: alex
Password: madeline
```

* **Attention**: After solving each section, ⛳️ flags will show up in: `http://$TARGET:1337`

## 🔎 Discovery

1. 🎯 First of all, Let's see what is going on this server: `nmap -p- -sV $TARGET`
```
Nmap scan report for $TARGET
Host is up (0.00024s latency).
Not shown: 65531 closed ports
PORT      STATE SERVICE VERSION
22/tcp    open  ssh     OpenSSH 7.9p1 Debian 10+deb10u2 (protocol 2.0)
80/tcp    open  http    Apache httpd 2.4.43 ((Unix))
1337/tcp  open  http    nginx 1.14.0 (Ubuntu)
65499/tcp open  ssh     OpenSSH 7.6p1 Ubuntu 4ubuntu0.3 (Ubuntu Linux; protocol 2.0)
MAC Address: 02:1C:A6:53:A7:95 (Unknown)
Service Info: OS: Linux; CPE: cpe:/o:linux:linux_kernel
```

2. 😵 Logging in SSH gives us a repeatitive sentence:
```sh
ssh alex#$TARGET
...
YOU DIDN'T SAY THE MAGIC WORD!
...
```

![Error](images/02.png)

3. ☹️ There is another SSH on port **65499** but we couldn't login.

![login error](images/03.png)

4. 😎 SSH could be Forced to execute another binary immediately after login. This helps logging in and Finding the malware.
```ssh
ssh alex@$TARGET '/bin/bash'
```

![home Files](images/04.png)

5. 🚀 `scp` is a suitable command for downloading it.
```
scp alex@$TARGET:/home/alex/fixutil ./fixutil
```

![scp file](images/05.png)

* **Attention**: Binaries of this challenge are already downloaded and included in this [Directory](files/).

## ⛳️ Flag 0: Fixing SSH Session

6. 🐞 Open `fixutil` File in **IDA Free** (There is a free version of [**IDA Pro**](https://hex-rays.com/ida-free))

![asm file](images/06-asm.png)

7. Pressing **F5** in IDA will show Us the **decompiled** code.

![c file](images/07-c.png)

8. Lines 6 to 8, show additon of a `while` loop into `.bashrc` and avoid loading `/bin/bash` on SSH login.
```
\n\nwhile :; do echo \"YOU DIDN'T SAY THE MAGIC WORD!\"; done &\n
```

![file bashrc](images/08-bashrc-edit.png)

09. Checking `.bashrc` file proves the assumption.

![bashrc](images/09-bashrc.png)

10. Lets remove **JUNK** lines, It'll help us login using **SSH** properly.
```sh
# cut top 114 lines in .bashrc and ditch modified ones
head -n 114 .bashrc > temp.rc
# check temp.rc file, if everything is OK!?
mv temp.rc .bashrc
```

![tail bashrc](images/10-tail-bashrc.png)

11. Fixing this issue, solves the SSH login problem and reveals the ⛳️ Flag 0 on: `http://$TARGET:1337/`

![ssh normal](images/11-ssh-normal.png)

## ⛳️ Flag 2: Replacing Libraries

12. Another Modified file is: `/lib/x86_64-linux-gnu/liblogging.so`

![file lib](images/12-lib.png)

13. But, File `/tmp/logging.so` does not exist. We could find it somewhere else:
```sh
ls -l /lib/x86_64-linux-gnu/ | grep log
-rwxrwxrwx 1 root root   23176 Jun 17  2020 liblogging.so
-rwxr-xr-x 1 alex alex   16048 Jun 17  2020 oldliblogging.so
```

14. `liblogging.so` is definietly another stage of malware, we can't recover the server without getting it using `scp`.
```
scp alex@$TARGET:/lib/x86_64-linux-gnu/liblogging.so ./liblogging.so
```

15. In the end of this section, `liblogging.so` should be replaced with OLD one (As step **13** shows, Current user has Permissions to do it).
```sh
cp /lib/x86_64-linux-gnu/oldliblogging.so /lib/x86_64-linux-gnu/liblogging.so
```

16. Flag 2 reveals in the panel: `http://$TARGET:1337/`

## ⛳️ Down to 🐰 Rabbit Hole 🕳️

17. Last Modified File is: `/bin/admin`

![admin file](images/17-admin-file.png)

18. Dumping it with `scp`.
```
scp alex@$TARGET:/bin/admin ./admin
```

19. RE this file and You'll See It has a Simple Password but It seems there is nothing to follow here!

![admin pass](images/19-pass-admin.png)

## ⛳️ Flag 1: Fix Accidental Session Closing

20. By analyzing `liblogging.so` more secrets reveal, It makes a **CRON JOB** in linux and writes `brilliant_script.sh` script and executes it periodically to close some applications such as `bash`. This is the reason Our **SSH** disconnects after short period of time.
```sh
cat /opt/brilliant_script.sh
#!/bin/sh

for i in $(ps aux | grep bash | grep -v grep | awk '{print $2}'); do kill $i; done;
```

![cron job](images/20-cron-job.png)

21. We have enough access to modify the File. By removing the last line of file, we can avoid closing **SSH sessions**.
```sh
# Removing Last Line
head -n 2 /opt/brilliant_script.sh > /opt/brilliant_script.sh
```

![disable bril](images/21-disable-brilliant.png)

22. This fixes the accidental session closing and Gives us ⛳️ Flag 1

## ⛳️ Flag 3: Privilege Escalation

23. Checking **Cron Job** in malware, shows much more hidden points, The Job executes the script as **ROOT** user.

![cron job](images/20-cron-job.png)

24. We can use an old trick, Adding a SSH Public to server, and using it to login as **ROOT**. Let's create a ssh key
```sh
# generating public/private keys
ssh-keygen
# you may choose another name for file!
cat id_rsa.pub
# ssh-rsa AAAA ... root@computer-name
```

![ssh key](images/24-gen-ssh-key.png)

25. change `computer-name` in the end of ssh key to `recovery` and use `/opt/brilliant_script.sh` to add your ssh key to root:
```ssh
echo 'echo "ssh-rsa AAAA...U= root@recovery" > /root/.ssh/authorized_keys' >> /opt/brilliant_script.sh
```

![add sshkey](images/25-add-ssh-key.png)

26. If you check the reverse engineered library, Malware adds a SSH Key to root same as what we did, But what we did in step 25, removes SSH Key of malware and gives us the ⛳️ Flag 3.

## ⛳️ Flag 4: Removing User

27. Now, Logging into server with private key is simple.
```
ssh -i id_rsa root@$TARGET
```

![ssh root](images/27-login-root.png)

28. REing the library file, shows addition of a user called: `security`

![user addition](images/28-adding-user.png)

29. For Flag 4 we need to Delete user: `userdel -f security`

![user del](images/29-userdel.png)

## ⛳️ Flag 5: Recovering Web Files

30. As description says, This is a ransomware, in this server, web Files are encrypted and we need to Find key.

31. As we go through malware in IDA. It seems backdoored library does the encryption job. **XOR** function is used for encryting web files. Randomly generated KEY is saved inside: `/opt/.fixutil/backup.txt` (LUCKY FOR US!)

![random](images/31-rand-str.png)
![ransome](images/31-xor-webfiles.png)

32. Normally I use `python` for decryption but in this case, it is not installed on the server. AI generated a conprehensive code in **C** for us with following command:
```
malware encrypted my files in: /usr/local/apache2/htdocs/
I found the key in: /opt/.fixutil/backup.txt
encryption algorithm is: xor
help me with a code in C to decrypt them
```

* **Attention**: `xor_decrypt.c` File is [included](Files/xor_decrypt.c).

33. Using `nano` or `vim` we could write the `xor_decrypt.c` file into the server and built it using **GCC** (AI gives the build instruction).

```sh
gcc -O2 -Wall -o xor_decrypt xor_decrypt.c -std=c11 -D_XOPEN_SOURCE=700

# Test and Dry run:
./xor_decrypt --key /opt/.fixutil/backup.txt --target /usr/local/apache2/htdocs --dry-run

# Actual Run
sudo ./xor_decrypt --key /opt/.fixutil/backup.txt --target /usr/local/apache2/htdocs
```

![xor c](images/33-xor-decrypt-c.png)

34. Checking `http://$TARGET:80` shows the website is back and this should Give us the Final ⛳️ Flag (5)

![web](images/34-website-up.png)
