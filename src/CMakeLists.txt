add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:DEBUG>")
