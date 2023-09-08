# 🍥🖥️ Home-Server

**Home-server** is a serlf made server writen in c++ for personal use.

## 📦 Installation
Install from terminal run
```shell
curl https://github.com/IJJA3141/Home-Server/releases/latest/download/script.zip -OL
unzip ./script.zip
rm ./script.zip
bash ./script.bash
```
Or from the realease page   
[Server](https://github.com/IJJA3141/Home-Server/releases/latest/download/server.zip)   
[Frontend](https://github.com/IJJA3141/Home-Server/releases/latest/download/frontend.zip)

Then you'll have to unzip both files.

## ⚙️ Setup
To start server use
```shell
./server/backend/server.out
```
You can modify the port onwitch the servers will listen and the path to the sertificate. By default the settings will always be saved.
### 📡 Commands
```
-?, --help          Shows help page
-n, --no-save       Disable parametters saving
-r, --reset         Run server with default parametters (you might want to use it with -n)
-h, --http-port     Change port onwitch the http sever will listen
-s, --https-port    Change port onwitch the https server will listen
-k, --key--path     Change path of certificate key file
-c, --cert--path    Change path of certificate file
```
## 🐛 Debug
### Error codes     
Parametters error:      
- 110 The user asked for help page
- 111 The user provided wrong commad or parametters           

Server error:
- 120 Server failed to create a socket
- 121 Server failed to applie options to socket
- 122 Server failed to bind to port
- 123 Server failed to listen     

SSL error:
- 130 Failed to load ssl context
- 131 Faild to load ssl certificate file
- 132 Faild to load ssl certificate key file
- 133 The certificate key provided didn't match the certificate       

## 📁 File structure
```tree
server/
    ├───backend/
    │   ├───data/
    │   │   ├───cert/
    │   │   │   ├───cert.pem
    │   │   │   └───key.pem
    │   │   └───settings.txt
    │   └───server.out
    └───frontend/
```