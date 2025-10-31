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

4. 😎 SSH could be Forced to execute another binary immediately after login. This causes logging in and Finding the malware.
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

