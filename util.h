#pragma once

#include <concepts>
#include <span>
#include <string>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <gsl/gsl>

namespace util
{
  inline std::span<const std::byte> to_data(std::string_view str) noexcept
  {
    GSL_SUPPRESS(type.1) {
      return std::span<const std::byte>(reinterpret_cast<const std::byte*>(str.data()), str.size());
    }
  }
  inline std::string_view to_strview(std::span<const std::byte> data) noexcept
  {
    GSL_SUPPRESS(type.1) {
      return std::string_view(reinterpret_cast<const char*>(data.data()), data.size());
    }
  }
  inline void* to_pointer(uintptr_t value) noexcept
  {
    GSL_SUPPRESS(type.1) GSL_SUPPRESS(lifetime.2) {
      return reinterpret_cast<void*>(value);
    }
  }
  inline uintptr_t to_int(void* ptr) noexcept
  {
    GSL_SUPPRESS(type.1) return reinterpret_cast<uintptr_t>(ptr);
  }
  constexpr std::byte to_byte(uint8_t value) noexcept
  {
    GSL_SUPPRESS(type.4) return std::byte{ value };
  }
}
