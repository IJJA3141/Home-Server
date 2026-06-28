#pragma once

// from https://en.cppreference.com/cpp/utility/variant/visit
#include <cstddef>
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

template <class... Ts> struct overloads : Ts...
{
  using Ts::operator()...;
};

inline std::string load_file(const std::filesystem::path& file)
{
  std::size_t size = std::filesystem::file_size(file);
  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) throw std::system_error(errno, std::generic_category(), "could not open file");

  char* addr = static_cast<char*>(mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0));

  if (close(fd))
  {
    // can't do much;
    // but mmap might still work
  }

  if (addr == MAP_FAILED) throw std::system_error(errno, std::generic_category(), "could not mmap");
  std::string str{addr, size};

  if (munmap(addr, size))
  {
    // can't do much;
    // but mmap might still work
  }

  return str;
}
