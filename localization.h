#pragma once

#include <map>
#include <string>
#include <string_view>

namespace locale
{
  std::string utf16_to_gbk(std::wstring_view s);
  std::wstring gbk_to_utf16(std::string_view s);
  std::string utf16_to_shiftjis(std::wstring_view s);
  std::wstring shiftjis_to_utf16(std::string_view s);
  std::wstring to_fullwidth(std::wstring_view s);
  std::wstring to_halfwidth(std::wstring_view s);

  void setup() noexcept;
}
