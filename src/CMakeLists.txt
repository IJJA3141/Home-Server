set(PROJECT_NAME reverse_proxy)
add_executable(${PROJECT_NAME} reverse_proxy/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

set(PROJECT_NAME mihon_sync)
add_executable(${PROJECT_NAME} mihon/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Debug>:DEBUG>")
