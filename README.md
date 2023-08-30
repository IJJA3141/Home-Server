# 🍥🖥️ Home-Server

**Home-server** is a serlf made server writen in c++ for personal use.

## 📦 Installation & ⚙️ setup

Install the script from the latest [release](https://github.com/IJJA3141/Home-Server/releases/latest/download/script.zip):
```bash
" curl
curl https://github.com/IJJA3141/Home-Server/releases/latest/download/script.zip -OL
unzip ./script.zip
```
Then run the script via bash:
```bash
" bash
bash ./script.sh
```
It should download the latest release and unzip everithing.
In the end it should look something like that:
```
/:  
│
└─ ~/server/
    │
    ├─ backend/
    │   │
    │   ├─ server.out
    │   ├─ cert/ 
    │   │   │     
    │   │   ├─ cert.pem
    │   │   └─ key.pem
    │   │
    │   └─ data/
    │       │
    │       └─ *
    │
    └─ frontend/
        │
        └─ *
```
To update it just rerun the script.

## 🚀 Usage

Server.out creates a http server on port 80 and an https server on port 443  by default. 
