cmake_minimum_required(VERSION 4.0.2) # cmake version
project(server)                       # name

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

set(LIBS -lpthread -lssl -lcrypto -luuid)     # libs
set(SOURCE_FILES                              # src
  ${CMAKE_SOURCE_DIR}/src/ipc/server.cpp
  ${CMAKE_SOURCE_DIR}/src/ipc/client.cpp

  ${CMAKE_SOURCE_DIR}/src/networking/auth.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/client.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/parse.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/route.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/router.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/tcp.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/tls.cpp
  ${CMAKE_SOURCE_DIR}/src/networking/unparse.cpp
  
  ${CMAKE_SOURCE_DIR}/src/protocol/http_deserialization.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/http_serialization.cpp
  ${CMAKE_SOURCE_DIR}/src/protocol/http_transition.cpp
)

enable_testing()
add_subdirectory(src)
add_subdirectory(test)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
)
