add_library(protocol STATIC
  http/http.cpp
)

target_include_directories(protocol PUBLIC include)
target_link_libraries(protocol PUBLIC
  common
)
