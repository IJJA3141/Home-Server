cmake_minimum_required(VERSION 3.25.1)
project(Home-Server)

set(SOURCE_FILES
    ./src/main.cpp
    ./src/tcp-server/tcp-server.cpp
    ./src/tls-server/tls-server.cpp
    ./src/client/client.cpp
    ./src/client/ssl-client.cpp
    ./src/parser/parser.cpp
    ./src/params/params.cpp
    )

set(INC_DIR /usr/local/include)
set(LINK_DIR /usr/local/lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
add_compile_definitions("DEBUG=$<CONFIG:Debug>")
ADD_EXECUTABLE(server.out ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(server.out -lpthread -lssl -lcrypto)
