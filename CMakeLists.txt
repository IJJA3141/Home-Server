cmake_minimum_required(VERSION 3.25.1)
project(Home-Server)

set(SOURCE_FILES
    ./src/main.cpp
    ./src/option/option.cpp
    ./src/test/test.cpp
    )

set(INC_DIR /usr/local/include)
set(LINK_DIR /usr/local/lib)

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_compile_definitions(DEBUG)
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
ADD_EXECUTABLE(server.out ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(server.out -lpthread -lssl -lcrypto)
