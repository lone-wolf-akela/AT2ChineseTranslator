#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>

#include <cwchar>
#include <cassert>

#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <fmt/format.h>

#include "util.h"

#include "memory_tool.h"

namespace 
{
  bool is_valid_page(const MEMORY_BASIC_INFORMATION& info) noexcept
  {
    return info.State == MEM_COMMIT &&
      info.Type == MEM_PRIVATE &&
      (info.Protect == PAGE_READWRITE || info.Protect == PAGE_READONLY);
  }
}

namespace memory
{
  bool Process::find_process(std::wstring_view process_name)
  {
    _process_name = process_name;
    _pid = 0;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry) == TRUE)
    {
      do
      GSL_SUPPRESS(bounds.3) {
        const auto l = wcsnlen_s(entry.szExeFile, sizeof(entry.szExeFile) / sizeof(wchar_t));
        if (std::wstring_view(entry.szExeFile, l) == process_name)
        {
          _pid = entry.th32ProcessID;
          break;
        }
      } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return _pid != 0;
  }
  void Process::open_process() noexcept
  {
    assert(_pid != 0);
    _handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, _pid);
    assert(_handle != nullptr);
  }
  void Process::close_process() noexcept
  {
    if (_handle != nullptr)
    {
      CloseHandle(_handle);
    }
  }
  void Process::acquire_pages()
  {
    assert(_handle != nullptr);

    MEMORY_BASIC_INFORMATION page{};
    std::byte* address = nullptr;
    
    _pages.clear();
    GSL_SUPPRESS(lifetime.1)
    while (VirtualQueryEx(_handle, address, &page, sizeof(page)) != 0)
    {
      if (is_valid_page(page))
      {
        _pages.emplace_back(util::to_int(page.BaseAddress), page.RegionSize);
      }
      GSL_SUPPRESS(bounds.1)
      address += page.RegionSize;
    }
  }
  std::vector<uintptr_t> Process::search(std::span<const std::byte> value)
  {
    std::vector<uintptr_t> addrs;
    for (const auto& page : _pages)
    {
      const auto data = _readpage(page);
      auto found = begin(data);
      GSL_SUPPRESS(lifetime.1) {
        while ((found = std::search(found, end(data), begin(value), end(value))) != end(data))
        {
          addrs.emplace_back(page.address + (found - begin(data)));
          if ((found = next(found)) == end(data)) { break; }
        }
      }
    }
    return addrs;
  }
  std::vector<std::byte> Process::read(uintptr_t addr, size_t len)
  {
    std::vector<std::byte> data(len);
    SIZE_T bytes_read;
    const auto r = ReadProcessMemory(_handle, util::to_pointer(addr), data.data(), len, &bytes_read);
    if (r == FALSE)
    {
      bytes_read = 0;
    }
    data.resize(bytes_read);
    return data;
  }
  std::vector<std::byte> Process::read_until(uintptr_t addr, std::byte endding, size_t max_len)
  {
    std::vector<std::byte> data;
    std::byte b{};
    SIZE_T bytes_read{};

    for (const uintptr_t p : ranges::views::iota(addr, addr + max_len))
    {
      const auto r = ReadProcessMemory(_handle, util::to_pointer(p), &b, 1, &bytes_read);
      if (r == FALSE || bytes_read == 0)
      {
        break;
      }
      data.emplace_back(b);
      if (b == endding)
      {
        break;
      }
    }
    return data;
  }
  Process::~Process()
  {
    close_process();
  }
  std::vector<std::byte> Process::_readpage(const MemPage& page)
  {
    return read(page.address, page.size);
  }
}
