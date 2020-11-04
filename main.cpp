#include <thread>
#include <chrono>
#include <string>
#include <string_view>

#include <fmt/format.h>

#include "memory_tool.h"
#include "localization.h"
#include "translation.h"

using namespace std::chrono_literals;
using namespace std::string_literals;

constexpr auto AnchorStr = L"Checking and loading saved data from the Memory Card ";

int main()
{
  locale::setup();
  translation::setup();

  fmt::print("{}\n", "Hello World!");
  memory::Process proc;
  fmt::print(L"正在寻找pcsx2.exe...\n");
  while (!proc.find_process(L"pcsx2.exe"))
  {
    std::this_thread::sleep_for(100ms);
  }
  fmt::print(L"已找到pcsx2.exe\n");
  proc.open_process();
  const auto fullwidth_anchor = locale::to_fullwidth(AnchorStr);
  auto shiftjis_anchor = locale::utf16_to_shiftjis(fullwidth_anchor);
  auto data_anchor = std::span<std::byte>(reinterpret_cast<std::byte*>(shiftjis_anchor.data()), shiftjis_anchor.size());

  std::vector<void*> p1;
  std::vector<void*> p2;
  fmt::print(L"等待游戏启动...\n");
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
  fmt::print(L"游戏已启动，寻找文本位置...\n");
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
  const auto addr = (p2.at(0) == p1.at(0)) ? p2.at(1) : p2.at(0);
  fmt::print(L"找到文本内存地址: 0x{:x}\n", size_t(addr));
  std::string str;
  while (true)
  {
    std::this_thread::sleep_for(100ms);
    auto data = proc.read_until(addr, std::byte(0));
    auto temp_str = std::string_view(reinterpret_cast<char*>(data.data()), data.size());
    if (str == temp_str)
    {
      continue;
    }
    str = std::move(temp_str);
    translation::translate(locale::shiftjis_to_utf16(str));
  }
  return 0;
}
