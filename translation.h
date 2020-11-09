#pragma once

#include <string>
#include <string_view>

#include "boost_hof_patched.h"

namespace translation
{
  void setup();
  void translate_impl(std::wstring_view str_english);
  constexpr auto translate = boost::hof::pipable(translate_impl);
}
