#include <thread>
#include <chrono>
#include <string>
#include <string_view>
#include <iostream>
#include <format>

#include "memory_tool.h"
#include "localization.h"
#include "translation.h"
#include "util.h"

using namespace std::chrono_literals;

constexpr auto AnchorStr = L"Checking and loading saved data from the Memory Card ";

int main()
{
  locale::setup();

  std::wcout << L"工具加载中，请稍后...\n";
  translation::setup();

  memory::Process proc;
  std::wcout << L"正在寻找pcsx2.exe...\n";
  while (!proc.find_process(L"pcsx2.exe"))
  {
    std::this_thread::sleep_for(100ms);
  }
  std::wcout << L"已找到pcsx2.exe\n";
  proc.open_process();
  const auto shiftjis_anchor = AnchorStr | locale::to_fullwidth | locale::utf16_to_shiftjis;
  const auto data_anchor = util::to_data(shiftjis_anchor);

  std::vector<uintptr_t> p1;
  std::vector<uintptr_t> p2;
  std::wcout << L"等待游戏启动...\n";
  while (true)
  {
    proc.acquire_pages();
    p2 = proc.search(data_anchor);
    if (p2.size() == 2)
    {
      break;
    }
    std::this_thread::sleep_for(100ms);
  }
  std::wcout << L"游戏已启动，寻找文本位置...\n";
  while (true)
  { 
    proc.acquire_pages();
    p1 = proc.search(data_anchor);
    if (p1.size() == 1)
    {
      break;
    }
    std::this_thread::sleep_for(100ms);
  }
  const auto textbox_addr = (p2.at(0) == p1.at(0)) ? p2.at(1) : p2.at(0);
  std::wcout << std::format(L"找到文本内存地址: 0x{:x}\n", size_t(textbox_addr));
  std::string str;
#if defined(_DEBUG)
  while (true)
  {
    std::wstring input;
    std::getline(std::wcin, input);
    if (input.substr(0, 2) == L"0x")
    {
      const uintptr_t addr = std::stoul(input);
      auto data = proc.read_until(addr, util::to_byte(0));
      auto temp_str = util::to_strview(data);
      std::wcout << std::format(L"{}\n", temp_str | locale::shiftjis_to_utf16 | locale::to_halfwidth);
      continue;
    }
    const auto shiftjis = input | locale::to_fullwidth | locale::utf16_to_shiftjis;
    const auto data = util::to_data(shiftjis);
    proc.acquire_pages();
    auto vec_p = proc.search(data);
    for (auto ptr : vec_p)
    {
      std::wcout << std::format(L"0x{:x}\n", size_t(ptr));
    }
  }
#endif
  while (true)
  {
    std::this_thread::sleep_for(100ms);
    auto data = proc.read_until(textbox_addr, util::to_byte(0));
    const auto temp_str = util::to_strview(data);
    if (str == temp_str)
    {
      continue;
    }
    str = temp_str;
    str | translation::replace_ctrlcode | locale::shiftjis_to_utf16 | translation::translate;
  }
  return 0;
}
