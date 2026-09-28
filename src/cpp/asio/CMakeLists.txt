add_library(asio STATIC
  socket_base.cpp
  executor.cpp
  reactor.cpp

  tcp/socket.cpp
  tcp/acceptor.cpp
)

# target_include_directories(common PUBLIC include)
# target_compile_features(common PUBLIC cxx_std_26)
target_compile_definitions(asio PUBLIC "$<$<CONFIG:Debug>:__DEBUG>")
