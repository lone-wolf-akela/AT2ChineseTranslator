#pragma once
#include <type_traits>

// std::is_literal_type is removed in c++20, so we have to add it back to use boost-hof
namespace std
{
  template <class _Ty>
  struct is_literal_type : bool_constant<__is_literal_type(_Ty)>
  {};

  template <class _Ty>
  constexpr bool is_literal_type_v = __is_literal_type(_Ty);
}

#include <boost/hof.hpp>
