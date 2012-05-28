// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BoostArray.hpp"
#include "common/StringConversion.hpp"

using namespace boost;
using namespace boost::detail::multi_array;

namespace cf3 {
namespace common {

template <>
Common_API std::string to_str< multi_array<Uint,2> > (const multi_array<Uint,2> & v)
{
  std::string s = "";
  if (v.num_elements()) {
    for (Uint i=0; i<v.size(); ++i) {
      for (Uint j=0; j<v[i].size(); ++j) {
        s += to_str(v[i][j]) + " ";
      }
      s += "\n";
    }
  }
  return s;
}

template <>
Common_API std::string to_str< multi_array<Real,2> > (const multi_array<Real,2> & v)
{
  std::string s = "";
  if (v.num_elements()) {
    for (Uint i=0; i<v.size(); ++i) {
      for (Uint j=0; j<v[i].size(); ++j) {
        s += to_str(v[i][j]) + " ";
      }
      s += "\n";
    }
  }
  return s;
}

template <>
Common_API std::string to_str< multi_array_view<Uint,2> > (const multi_array_view<Uint,2> & v)
{
  std::string s = "";
  if (v.num_elements()) {
    for (Uint i=0; i<v.size(); ++i) {
      for (Uint j=0; j<v[i].size(); ++j) {
        s += to_str(v[i][j]) + " ";
      }
      s += "\n";
    }
  }
  return s;
}

template <>
Common_API std::string to_str< multi_array_view<Real,2> > (const multi_array_view<Real,2> & v)
{
  std::string s = "";
  if (v.num_elements()) {
    for (Uint i=0; i<v.size(); ++i) {
      for (Uint j=0; j<v[i].size(); ++j) {
        s += to_str(v[i][j]) + " ";
      }
      s += "\n";
    }
  }
  return s;
}

template <>
Common_API std::string to_str< sub_array<Real, 1> > (const sub_array<Real, 1>& v)
{
  std::string s = "";
  if (v.num_elements()) {
    for (Uint i=0; i<v.size(); ++i) {
      if (i!=0) s+= " ";
      s += to_str(v[i]);
    }
  }
  return s;

}

} // namespace common
} // namespace cf3

