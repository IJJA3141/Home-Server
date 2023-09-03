#!/bin/bash

mkdir -p ./server/backend/data ./server/backend/cert ./server/frontend
curl https://github.com/IJJA3141/Home-Server/releases/latest/download/server.zip -LO
curl https://github.com/IJJA3141/Home-Server/releases/latest/download/frontend.zip -LO
unzip -u ./server.zip -d ./server/backend/
unzip -u ./frontend.zip -d ./server/frontend/
rm ./server.zip ./frontend.zip
