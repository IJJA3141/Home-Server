add_library(common STATIC
  logger.cpp
)

# target_include_directories(common PUBLIC include)
# target_compile_features(common PUBLIC cxx_std_26)
find_library(UUID_LIB uuid REQUIRED)
target_link_libraries(common PUBLIC ${UUID_LIB})
target_compile_definitions(common PUBLIC "$<$<CONFIG:Debug>:__DEBUG>")
