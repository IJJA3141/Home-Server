#pragma once

#include <chrono>
#include <filesystem>

constexpr size_t session_cache_size = 5;
constexpr auto session_ttl = std::chrono::hours{3 * 24};
