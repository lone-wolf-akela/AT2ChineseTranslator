#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cassert>
#include <cstdio>

#include <io.h>
#include <fcntl.h>

#include <range/v3/all.hpp>

#include "localization.h"

namespace
{
  std::string utf16_to_ansi(std::wstring_view s, UINT codepage)
  {
    const auto l = WideCharToMultiByte(
      codepage,
      0,
      reinterpret_cast<LPCWCH>(s.data()),
      s.size(),
      nullptr,
      0,
      nullptr,
      nullptr
    );
    assert(s.size() == 0 || l != 0);
    std::string ansistr(l, '\0');
    WideCharToMultiByte(
      codepage,
      0,
      reinterpret_cast<LPCWCH>(s.data()),
      s.size(),
      ansistr.data(),
      ansistr.size(),
      nullptr,
      nullptr
    );
    return ansistr;
  }
  std::wstring ansi_to_utf16(std::string_view s, UINT codepage)
  {
    const auto l = MultiByteToWideChar(
      codepage,
      0,
      s.data(),
      s.size(),
      nullptr,
      0
    );
    assert(s.size() == 0 || l != 0);
    std::wstring u16str(l, L'\0');
    MultiByteToWideChar(
      codepage,
      0,
      s.data(),
      s.size(),
      reinterpret_cast<LPWSTR>(u16str.data()),
      u16str.size()
    );
    return u16str;
  }

  std::wstring width_conv(std::wstring_view s, DWORD action)
  {
    const auto l = LCMapStringEx(
      LOCALE_NAME_SYSTEM_DEFAULT,
      action,
      s.data(),
      s.size(),
      nullptr,
      0,
      nullptr,
      nullptr,
      0
    );
    assert(s.size() == 0 || l != 0);
    std::wstring result(l, L'\0');
    LCMapStringEx(
      LOCALE_NAME_SYSTEM_DEFAULT,
      action,
      s.data(),
      s.size(),
      result.data(),
      result.size(),
      nullptr,
      nullptr,
      0
    );
    if (result.back() == L'\0')
    {
      result.pop_back();
    }
    return result;
  }
}

namespace locale
{
  std::string utf16_to_gbk_impl(std::wstring_view s)
  {
    return utf16_to_ansi(s, 54936);
  }
  std::wstring gbk_to_utf16_impl(std::string_view s)
  {
    return ansi_to_utf16(s, 54936);
  }
  std::string utf16_to_shiftjis_impl(std::wstring_view s)
  {
    return utf16_to_ansi(s, 932);
  }

  std::wstring shiftjis_to_utf16_impl(std::string_view s)
  {
    return ansi_to_utf16(s, 932);
  }

  std::wstring to_fullwidth_impl(std::wstring_view s)
  {
    return width_conv(s, LCMAP_FULLWIDTH);
  }

  std::wstring to_halfwidth_impl(std::wstring_view s)
  {
    return width_conv(s, LCMAP_HALFWIDTH);
  }

  void setup() noexcept
  {
    const auto old_mode = _setmode(_fileno(stdout), _O_U16TEXT);
    assert(old_mode != -1);
  }
}
