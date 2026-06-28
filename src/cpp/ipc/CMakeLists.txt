add_library(ipc STATIC
  client.cpp
  server.cpp
)

target_compile_definitions(ipc PRIVATE "$<$<CONFIG:Debug>:__DEBUG>")
target_include_directories(ipc PUBLIC include)
target_link_libraries(ipc PUBLIC
  common
)
