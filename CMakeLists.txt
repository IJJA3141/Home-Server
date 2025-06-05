cmake_minimum_required(VERSION 3.29)
project(Home-Server)

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_compile_definitions(DEBUG)
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

set(INC_DIR /usr/local/include)
set(LINK_DIR /usr/local/lib)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})

set(SOURCE_FILES ./src/main.cpp)
add_executable(server.out ${SOURCE_FILES})
target_link_libraries(server.out -lpthread -lssl -lcrypto)

enable_testing()
set(TEST_FILES ./test/main.cpp ./test/request.cpp)
add_executable(test.out ${TEST_FILES})
add_test(Tester test.out)
