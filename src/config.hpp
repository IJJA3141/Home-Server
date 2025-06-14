#pragma once

#include <chrono>
#include <filesystem>

const std::filesystem::path cwd("~/tmp");

constexpr size_t session_cache_size = 5;
constexpr auto session_ttl = std::chrono::hours{3 * 24};
