//---------------------------------------------------------------------------//
// Copyright (c) 
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
//---------------------------------------------------------------------------//
// bitfiled flags for enums
#include <type_traits>

#define BOOST_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
using T = std::underlying_type_t <ENUMTYPE>; \
inline ENUMTYPE operator~ (ENUMTYPE a) { return (ENUMTYPE)~(T)a; } \
inline ENUMTYPE operator| (ENUMTYPE a, ENUMTYPE b) { return (ENUMTYPE)((T)a | (T)b); } \
inline ENUMTYPE operator& (ENUMTYPE a, ENUMTYPE b) { return (ENUMTYPE)((T)a & (T)b); } \
inline ENUMTYPE operator^ (ENUMTYPE a, ENUMTYPE b) { return (ENUMTYPE)((T)a ^ (T)b); } \
inline ENUMTYPE& operator|= (ENUMTYPE& a, ENUMTYPE b) { return (ENUMTYPE&)((T&)a |= (T)b); } \
inline ENUMTYPE& operator&= (ENUMTYPE& a, ENUMTYPE b) { return (ENUMTYPE&)((T&)a &= (T)b); } \
inline ENUMTYPE& operator^= (ENUMTYPE& a, ENUMTYPE b) { return (ENUMTYPE&)((T&)a ^= (T)b); } \
}
#else
#define BOOST_ENUM_FLAG_OPERATORS(ENUMTYPE) 
#endif 