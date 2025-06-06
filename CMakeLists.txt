cmake_minimum_required(VERSION 4.0.2) # cmake version 
project(server)                       # name

set(CMAKE_CXX_STANDARD 26)            # cpp version

set(LIBS -lpthread -lssl -lcrypto -luuid)    # libs
set(SOURCE_FILES src/main.cpp         # src
)

#===================================#==========#
set(INC_DIR /usr/local/include)
set(LINK_DIR /usr/local/lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} "${CMAKE_SOURCE_DIR}/bin")

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_compile_definitions(DEBUG)
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

include(CTest)
enable_testing()

add_subdirectory(test)
