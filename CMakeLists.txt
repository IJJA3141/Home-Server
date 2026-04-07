cmake_minimum_required(VERSION 4.0.2) # cmake version 
project(server)                       # name

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(LIBS -lpthread -lssl -lcrypto -luuid)     # libs
set(SOURCE_FILES                              # src
  # lib
  ${CMAKE_SOURCE_DIR}/src/network/auth.cpp

  ${CMAKE_SOURCE_DIR}/src/network/tcp.cpp
  ${CMAKE_SOURCE_DIR}/src/network/tls.cpp
  ${CMAKE_SOURCE_DIR}/src/network/client.cpp

  ${CMAKE_SOURCE_DIR}/src/network/parse.cpp
  ${CMAKE_SOURCE_DIR}/src/network/unparse.cpp

  ${CMAKE_SOURCE_DIR}/src/network/router.cpp
  ${CMAKE_SOURCE_DIR}/src/network/route.cpp

  # imp
  ${CMAKE_SOURCE_DIR}/src/implementation/file.cpp
  ${CMAKE_SOURCE_DIR}/src/implementation/error_res.cpp
)

enable_testing()
add_subdirectory(src)
add_subdirectory(test)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
)
