add_executable(ikipittari
  main.cpp
)

target_compile_definitions(ikipittari PRIVATE "$<$<CONFIG:Debug>:__DEBUG>")
target_link_libraries(ikipittari PRIVATE
  common
  ipc
)
