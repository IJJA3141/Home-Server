add_executable(auth_agent
  main.cpp
)

target_link_libraries(auth_agent PRIVATE
  common
  ipc
)
