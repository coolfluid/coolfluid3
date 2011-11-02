// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Table_fwd_hpp
#define cf3_common_Table_fwd_hpp

#define BOOST_MULTI_ARRAY_NO_GENERATORS 0
#include <boost/multi_array/base.hpp>
#undef BOOST_MULTI_ARRAY_NO_GENERATORS

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Table;

template <typename T>
struct TableArray
{
  typedef boost::multi_array<T,2> type;
};


template <typename T>
struct TableRow
{
  typedef boost::detail::multi_array::sub_array<T,1> type;
};

template <typename T>
struct TableConstRow
{
  typedef const boost::detail::multi_array::const_sub_array<T,1> type;
};

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Table_fwd_hpp
