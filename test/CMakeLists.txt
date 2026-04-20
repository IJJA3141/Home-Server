set(TEST_FILES
  protocol/http.cpp
)

add_executable(unit_tests)

target_link_libraries(unit_tests PRIVATE ${LIBS})
target_compile_definitions(unit_tests PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

create_test_sourcelist(unit_tests unit_tests.cpp ${TEST_FILES})
target_sources(unit_tests PRIVATE ${SOURCE_FILES} ${TEST_FILES} unit_tests)

foreach(test ${TEST_FILES})
  get_filename_component(name ${test} NAME_WE)
  get_filename_component(path ${test} DIRECTORY)

  if(NOT(path STREQUAL ""))
    set(name "${path}/${name}")
  endif()

  add_test(
    NAME ${name}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    COMMAND "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test/unit_tests" ${name}
  )
endforeach()
