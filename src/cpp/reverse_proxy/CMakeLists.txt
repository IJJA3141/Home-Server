add_executable(reverse_proxy
  main.cpp
  forward_policy.cpp
  tcp_server.cpp
  tcp_client.cpp
  tls_server.cpp
)

target_link_libraries(reverse_proxy PRIVATE
  common
  ipc
)
