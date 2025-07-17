# Industrial Intrusion

* [Industrial Intrusion](https://tryhackme.com/room/industrial-intrusion)

## 1. Intro and Rules

Jus Read The Rules ...

## 2. Teams Assemble

Be **Bold** and Create a Team or just **join** one!

## 3. Breach

1. Scan with nmap: `nmap -p- -sV $TARGET`
```
22/tcp    open  ssh           OpenSSH 9.6p1 Ubuntu 3ubuntu13.11 (Ubuntu Linux; protocol 2.0)
80/tcp    open  http          Werkzeug/3.1.3 Python/3.12.3
102/tcp   open  iso-tsap      Siemens S7 PLC
502/tcp   open  mbap?
1880/tcp  open  vsat-control?
8080/tcp  open  http-proxy    Werkzeug/2.3.7 Python/3.12.3
44818/tcp open  EtherNetIP-2?
```
2. Interesting Ports
    * `80` a web UI for **flag**
    * `502` being used for **modbus** over TCP
    * `1880` being used for Node-RED, web-based work flow management

3. Check Node-RED application and debug log
    * **Coil 20** is used for **Motion Detector**
    * **Coil 25** is used for **Gate\Badge Checker**

4. We need to set coils on **20** and **25** to **ZERO** to **disable** the Gate. We can use `pymodbus` library in `python` to communicate with port `502` on **target**.

5. Install dependencies and run exploit
```sh
python3 -m pip install pymodbus=3.6.9
python3 ./03-breach/exploit.py $TARGET
```

7. Check for **changes** in **Node-RED** (logs, Motion Detector and Badge Checker)

6. Check for flag on browser: `http://$TARGET/`
```
THM{s4v3_th3_d4t3_27_jun3}
```
