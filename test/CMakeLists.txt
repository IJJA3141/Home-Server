set(TEST_FILES                      # test
  session.cpp

  parsing/method.cpp
  parsing/querys.cpp
  parsing/url.cpp
  parsing/request.cpp
)

set(SOURCE_FILES                    # src
  ../src/network/auth.cpp
  ../src/network/tcp.cpp
  ../src/network/tls.cpp
  ../src/network/client.cpp
  ../src/network/parser.cpp
)

#===================================#==========#
include(CTest)
enable_testing()

create_test_sourcelist(tests tests.cxx ${TEST_FILES})

add_executable(unit_tests ${tests} ${SOURCE_FILES})
target_link_libraries(unit_tests ${LIBS})

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_compile_definitions(DEBUG)
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

FOREACH(test ${TEST_FILES})
  get_filename_component(name ${test} NAME_WE)
  add_test(NAME ${name} COMMAND "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test/unit_tests"  ${name})
ENDFOREACH()
