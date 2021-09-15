#pragma once

#include <string>
#include <string_view>

#include "boost_hof_patched.h"

namespace translation
{
  void setup();
  void translate_impl(std::wstring_view str_english);
  std::string replace_ctrlcode_impl(std::string_view str);
  std::wstring remove_ctrlstr_impl(std::wstring_view str);
  std::wstring remove_nonalphabet_impl(std::wstring_view str);
  std::wstring prettify_ctrlstr_impl(std::wstring_view str);
  constexpr auto translate = boost::hof::pipable(translate_impl);
  constexpr auto replace_ctrlcode = boost::hof::pipable(replace_ctrlcode_impl);
  constexpr auto remove_ctrlstr = boost::hof::pipable(remove_ctrlstr_impl);
  constexpr auto remove_nonalphabet = boost::hof::pipable(remove_nonalphabet_impl);
  constexpr auto prettify_ctrlstr = boost::hof::pipable(prettify_ctrlstr_impl);
}
