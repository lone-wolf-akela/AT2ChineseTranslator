#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tlhelp32.h>

#include <cwchar>
#include <cassert>

#include <gsl/gsl>
#include <range/v3/all.hpp>
#include <fmt/format.h>

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
        _pages.emplace_back(page.BaseAddress, page.RegionSize);
      }
      GSL_SUPPRESS(bounds.1)
      address += page.RegionSize;
    }
  }
  std::vector<void*> Process::search(std::span<const std::byte> value)
  {
    std::vector<void*> addrs;
    for (const auto& page : _pages)
    {
      const auto data = _readpage(page);
      auto found = begin(data);
      while ((found = std::search(found, end(data), begin(value), end(value))) != end(data))
      {
        addrs.emplace_back(static_cast<std::byte*>(page.address) + (found - begin(data)));
        found++;
      }
    }
    return addrs;
  }
  std::vector<std::byte> Process::read(void* addr, size_t len)
  {
    std::vector<std::byte> data(len);
    SIZE_T bytes_read;
    const auto r = ReadProcessMemory(_handle, addr, data.data(), len, &bytes_read);
    if (r == FALSE)
    {
      bytes_read = 0;
    }
    data.resize(bytes_read);
    return data;
  }
  std::vector<std::byte> Process::read_until(void* addr, std::byte endding)
  {
    std::vector<std::byte> data;
    std::byte b;
    SIZE_T bytes_read;
    do
    {
      const auto r = ReadProcessMemory(_handle, addr, &b, 1, &bytes_read);
      if (r == FALSE)
      {
        break;
      }
      data.emplace_back(b);
      addr = static_cast<std::byte*>(addr) + 1;
    } while (bytes_read != 0 && b != endding);
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
