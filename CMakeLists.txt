cmake_minimum_required(VERSION 3.25.1)
project(Home-Server)

set(SOURCE_FILES
    ./src/main.cpp
    ./src/tcp-server/tcp-server.cpp
    )

  add_executable(server ${SOURCE_FILES})
