#pragma once

#include <string>
#include <string_view>

namespace translation
{
  void setup();
  void translate(std::wstring_view str_english);
}
