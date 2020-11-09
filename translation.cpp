#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <utility>
#include <vector>
#include <stdexcept>

#include <fmt/format.h>
#include <gsl/gsl>

#include "console.h"
#include "localization.h"
#include "translation.h"

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
  std::map<Index, std::wstring> idx_to_cn;

  void str_trim(std::string& str, std::string_view ending)
  {
    while (str.ends_with(ending))
    {
      str = str.substr(0, str.length() - ending.length());
    }
  }

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
        
        if (in_string && 3 == sscanf_s(temp.data(), "#### %d <#JMP($%x,$%x)>####", &idx, &p_from, &p_to))
        {
          emplace_data(data, text, Index{ filename, p_from });
          text = "";
          in_string = false;
        }

        if (!in_string)
        {
          if (temp.front() != '#')
          {
            continue;
          }
          const auto n_read = sscanf_s(temp.data(), "#### %d <#JMP($%x,$%x)>####", &idx, &p_from, &p_to);
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
  void setup()
  {
    auto en_data = load_data("us-text");
    for (auto&& [addr, text] : en_data)
    {
      en_to_idx.emplace(std::move(text), std::move(addr));
    }
    auto cn_data = load_data("cn-text");
    for (auto&& [addr, text] : cn_data)
    {
      idx_to_cn.emplace(std::move(addr), std::move(text));
    }
  }

  void translate_impl(std::wstring_view str_english)
  {
    console::clear();
    const auto halfwidth = locale::to_halfwidth(str_english);
    fmt::print(L"{}\n", halfwidth);
    
    const auto en_str_fullw = locale::to_fullwidth(str_english);
    const auto found_en = en_to_idx.find(en_str_fullw);
    if (found_en == end(en_to_idx))
    {
      fmt::print(L"英文句库中未找到此句\n");
      return;
    }
    const Index idx = found_en->second;
    const auto found_cn = idx_to_cn.find(idx);
    if (found_cn == end(idx_to_cn))
    {
      fmt::print(L"中文句库中未找到此句\n");
      return;
    }
    fmt::print(L"{}\n", found_cn->second);
  }
}
