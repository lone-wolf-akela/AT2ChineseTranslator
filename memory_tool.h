#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <span>

namespace memory
{
  using pid_t = uint32_t;
  using phandle_t = void*;

  struct MemPage
  {
    void* address;
    size_t size;
  };

  class Process
  {
  public:
    bool find_process(std::wstring_view process_name);
    void open_process() noexcept;
    void close_process() noexcept;
    void acquire_pages();
    std::vector<void*> search(std::span<const std::byte> value);
    std::vector<std::byte> read(void* addr, size_t len);
    std::vector<std::byte> read_until(void* addr, std::byte endding);

    Process() = default;
    Process(const Process&) = delete;
    Process(Process&&) = delete;
    Process& operator=(const Process&) = delete;
    Process& operator=(Process&&) = delete;
    ~Process();
  private:
    std::vector<std::byte> _readpage(const MemPage& page);

    std::wstring _process_name = L"";
    pid_t _pid = 0;
    phandle_t _handle = nullptr;
    std::vector<MemPage> _pages = {};
  };
}
