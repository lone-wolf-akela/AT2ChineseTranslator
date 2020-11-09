#pragma once

#include <map>
#include <string>
#include <string_view>

#include "boost_hof_patched.h"

namespace locale
{
  std::string utf16_to_gbk_impl(std::wstring_view s);
  std::wstring gbk_to_utf16_impl(std::string_view s);
  std::string utf16_to_shiftjis_impl(std::wstring_view s);
  std::wstring shiftjis_to_utf16_impl(std::string_view s);
  std::wstring to_fullwidth_impl(std::wstring_view s);
  std::wstring to_halfwidth_impl(std::wstring_view s);

  constexpr auto utf16_to_gbk = boost::hof::pipable(utf16_to_gbk_impl);
  constexpr auto gbk_to_utf16 = boost::hof::pipable(gbk_to_utf16_impl);
  constexpr auto utf16_to_shiftjis = boost::hof::pipable(utf16_to_shiftjis_impl);
  constexpr auto shiftjis_to_utf16 = boost::hof::pipable(shiftjis_to_utf16_impl);
  constexpr auto to_fullwidth = boost::hof::pipable(to_fullwidth_impl);
  constexpr auto to_halfwidth = boost::hof::pipable(to_halfwidth_impl);

  void setup() noexcept;
}
