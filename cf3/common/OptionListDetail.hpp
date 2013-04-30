// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file OptionListDetail.hpp

#ifndef cf3_common_OptionListDetail_hpp
#define cf3_common_OptionListDetail_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"

namespace cf3 {
namespace common {

// Some implementation details to help OptionList
/// Helper to choose the appropriate return type
/// Return OptionT<T> by default
template<typename T>
struct SelectOptionType
{
  typedef OptionT<T> type;
};

/// Allow string constants without enclosing them in std::string()
template<std::size_t N>
struct SelectOptionType< char[N] >
{
  typedef OptionT<std::string> type;
};

template<>
struct SelectOptionType< char const * >
{
  typedef OptionT<std::string> type;
};

/// Specialization for URI
template<>
struct SelectOptionType<URI>
{
  typedef OptionURI type;
};

/// Specialization for arrays
template<typename T>
struct SelectOptionType< std::vector<T> >
{
  typedef OptionArray<T> type;
};

/// Specialization for components
template<typename T>
struct SelectOptionType< Handle<T> >
{
  typedef OptionComponent<T> type;
};

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionListDetail_hpp
