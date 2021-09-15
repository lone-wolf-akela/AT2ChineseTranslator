#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <utility>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <format>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <optional>

#include <gsl/gsl>
#include <boost/algorithm/string/replace.hpp>
#include <range/v3/all.hpp>

#include "console.h"
#include "localization.h"
#include "translation.h"

using namespace std::literals;

namespace
{
  struct Index
  {
    std::string file{};
    size_t addr{};
    bool operator<(const Index& rhs) const noexcept
    {
      return (file == rhs.file) ? (addr < rhs.addr) : (file < rhs.file);
    }
  };
  std::map<std::wstring, Index> en_to_idx;
  std::map<std::wstring, Index> en_to_idx_no_ctrl;
  std::map<Index, std::wstring> idx_to_cn;

  void str_trim(std::string& str, std::string_view ending)
  {
    while (str.ends_with(ending))
    {
      str = str.substr(0, str.length() - ending.length());
    }
  }

  const std::map<std::string_view, std::string_view> control_tbl = {
    {"\xFA\x57"sv, "{EMPH}"sv},
    {"\x23\x30"sv, "{BRSET}"sv},
    {"\x23\x23"sv, "{BREND}"sv},
    {"\x43\x52"sv, "{CR}"sv},
    {"\x7B\x73\x7D"sv, "{S}"sv},
    {"\x49\x50\x44"sv, "{IPD}"sv},
    {"\x43\x4C\x59\x4C"sv, "{CL:594C}"sv},
    {"\x43\x4C\x4E\x52"sv, "{CL:4E52}"sv},
    {"\x43\x4C\x45\x47"sv, "{CL:4547}"sv},
    {"\x43\x4C\x42\x4B"sv, "{CL:424B}"sv},
    {"\x43\x4C\x42\x4C"sv, "{CL:424C}"sv},
    {"\x43\x4C\x42\x52"sv, "{CL:4252}"sv},
    {"\x43\x4C\x52\x31"sv, "{CL:5231}"sv},
    {"\x43\x4C\x52\x45"sv, "{CL:5245}"sv},
    {"\x43\x4C\x47\x52"sv, "{CL:4752}"sv},
    {"\x49\x4D\x30\x30"sv, "{IM:3030}"sv},
    {"\x49\x4D\x30\x31"sv, "{IM:3031}"sv},
    {"\x49\x4D\x30\x32"sv, "{IM:3032}"sv},
    {"\x49\x4D\x30\x33"sv, "{IM:3033}"sv},
    {"\x49\x4D\x30\x34"sv, "{IM:3034}"sv},
    {"\x49\x4D\x30\x35"sv, "{IM:3035}"sv},
    {"\x49\x4D\x30\x36"sv, "{IM:3036}"sv},
    {"\x49\x4D\x30\x37"sv, "{IM:3037}"sv},
    {"\x49\x4D\x30\x38"sv, "{IM:3038}"sv},
    {"\x49\x4D\x30\x42"sv, "{IM:3042}"sv},
    {"\x49\x4D\x55\x53"sv, "{IM:5553}"sv},
    {"\x49\x4D\x50\x4C"sv, "{IM:504C}"sv},
    {"\x53\x49\x50\x53\x30\x31"sv, "{SIPS:3031}"sv},
    {"\x45\x56\x49\x43\x30\x38"sv, "{EVIC:3038}"sv},
    {"\x45\x56\x49\x43\x30\x39"sv, "{EVIC:3039}"sv},
    {"\x45\x56\x49\x43\x30\x41"sv, "{EVIC:3041}"sv},
    {"\x45\x56\x49\x43\x30\x42"sv, "{EVIC:3042}"sv},
    {"\x45\x56\x49\x43\x30\x43"sv, "{EVIC:3043}"sv},
    {"\x7B\x45\x56\x43\x46\x5F\x32\x37\x7D"sv, "{EVCF_27}"sv},
    {"\x7B\x45\x56\x43\x46\x5F\x32\x38\x7D"sv, "{EVCF_28}"sv},
  };

  const std::map<std::wstring_view, std::wstring_view> ctrl_prettify_tbl =
  {
    {L"{EMPH}"sv, L"\""sv},
    {L"{CR}"sv, L"\n"sv},
    {L"{SIPS:3031}"sv, L"®"sv},
  };

  void emplace_data(std::vector<std::pair<Index, std::wstring>>& data, std::string text, Index idx)
  {
    str_trim(text, "\r\n");
    str_trim(text, "{end}");
    auto wtext = text | locale::gbk_to_utf16 | locale::to_fullwidth;
    data.emplace_back(std::make_pair(std::move(idx), std::move(wtext)));
  }

  std::vector<std::pair<Index, std::wstring>> load_data(std::string_view folder)
  {
    std::vector<std::pair<Index, std::wstring>> data;

    GSL_SUPPRESS(lifetime.1)
    for (const auto& p : std::filesystem::directory_iterator(folder))
    {
      std::ifstream ifs(p.path());
      assert(ifs);

      auto filename = p.path().stem().string();
      const auto found = filename.find('_');
      if (found != filename.npos)
      {
        filename = filename.substr(0, found);
      }
      int idx = 0;
      size_t p_from = 0, p_to = 0;
      std::string text;
      bool in_string = false;
      while (!ifs.eof() && ifs.peek() != EOF)
      {
        std::string temp;
        std::getline(ifs, temp);
        
        int new_idx{};
        size_t new_p_from{}, new_p_to{};
        if (in_string && 3 == sscanf_s(temp.data(), "#### %d <#JMP($%zx,$%zx)>####", &new_idx, &new_p_from, &new_p_to))
        {
          emplace_data(data, text, Index{ filename, p_from });
          text = "";
          in_string = false;

          idx = new_idx;
          p_from = new_p_from;
          p_to = new_p_to;
        }

        if (!in_string)
        {
          if (temp.front() != '#')
          {
            continue;
          }
          const auto n_read = sscanf_s(temp.data(), "#### %d <#JMP($%zx,$%zx)>####", &idx, &p_from, &p_to);
          assert(n_read == 3);
          in_string = true;
        }
        else
        {
          text += temp;
          text += "\r\n";
        } 
      }
      emplace_data(data, text, Index{ filename, p_from });
    }
    return data;
  }
}

namespace translation
{
  std::wstring prettify_ctrlstr_impl(std::wstring_view str)
  {
    std::wstring res{ str };
    for (const auto [from, to] : ctrl_prettify_tbl)
    {
      boost::replace_all(res, from, to);
    }
    return res;
  }

  std::string replace_ctrlcode_impl(std::string_view str)
  {
    std::string res{ str };
    for (const auto [from, to] : control_tbl)
    {
      boost::replace_all(res, from, to);
    }
    return res;
  }

  std::wstring remove_ctrlstr_impl(std::wstring_view str)
  {
    std::wstring res{ str };
    for (const auto [ctrlcode, ctrlstr] : control_tbl)
    {
      std::ignore = ctrlcode;
      const auto wide_ctrlstr = ctrlstr | locale::gbk_to_utf16 | locale::to_fullwidth;
      boost::replace_all(res, wide_ctrlstr, L"");
    }
    return res;
  }

  std::wstring remove_nonalphabet_impl(std::wstring_view str)
  {
    auto res = str | ranges::views::filter([](wchar_t c) {
      return c == u' ' || c == u'　' ||
        (u'a' <= c && c <= u'z') || (u'ａ' <= c && c <= u'ｚ') ||
        (u'A' <= c && c <= u'Z') || (u'Ａ' <= c && c <= u'Ｚ') ||
        (u'0' <= c && c <= u'9') || (u'０' <= c && c <= u'９');
      })
      | ranges::to<std::wstring>;
    return res;
  }

  void setup()
  {
    auto en_data = load_data("us-text");
    for (auto&& [addr, text] : en_data)
    {
      auto text_no_ctrlstr = text | remove_ctrlstr | remove_nonalphabet;
      en_to_idx_no_ctrl.emplace(std::move(text_no_ctrlstr), addr);
      en_to_idx.emplace(std::move(text), std::move(addr));
    }
    auto cn_data = load_data("cn-text");
    for (auto&& [addr, text] : cn_data)
    {
      idx_to_cn.emplace(std::move(addr), std::move(text));
    }
  }

  std::optional<Index> find_en_idx(const std::wstring& str)
  {
    if (const auto found = en_to_idx.find(str); found != end(en_to_idx))
    {
      return found->second;
    }
    const auto ctrlstr_removed = str | remove_ctrlstr | remove_nonalphabet;
    if (const auto found = en_to_idx_no_ctrl.find(ctrlstr_removed); found != end(en_to_idx_no_ctrl))
    {
      return found->second;
    }
    return std::nullopt;
  }

  void translate_impl(std::wstring_view str_english)
  {
    console::clear();
    const auto halfwidth = locale::to_halfwidth(str_english);
    std::wcout << std::format(L"{}\n", halfwidth | prettify_ctrlstr);
    
    const auto en_str_fullw = locale::to_fullwidth(str_english);
    const auto idx = find_en_idx(en_str_fullw);
    if (!idx.has_value())
    {
      std::wcout << L"英文句库中未找到此句\n";
      return;
    }
    const auto found_cn = idx_to_cn.find(*idx);
    if (found_cn == end(idx_to_cn))
    {
      std::wcout << L"中文句库中未找到此句\n";
      return;
    }
    std::wcout << L"====================\n";
    std::wcout << std::format(L"{}\n", found_cn->second | locale::to_halfwidth | prettify_ctrlstr);
  }
}
