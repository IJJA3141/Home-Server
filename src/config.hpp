#pragma once

#include <filesystem>

const auto CERT_PATH = std::filesystem::path("/home/alexe/tmp/cert.pem");
const auto KEY_PATH = std::filesystem::path("/home/alexe/tmp/key.pem");

#define MIHON_SYNC_PORT 23
#define BUFFER_SIZE 4096
#define LOCAL_HOST "127.0.0.0"
