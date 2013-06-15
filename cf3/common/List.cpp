// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "common/LibCommon.hpp"
#include "common/List.hpp"

namespace cf3 {
namespace common {

//////////////////////////////////////////////////////////////////////

common::ComponentBuilder < List<bool>, Component, LibCommon > List_bool_Builder;

common::ComponentBuilder < List<Uint>, Component, LibCommon > List_Uint_Builder;

common::ComponentBuilder < List<int>, Component, LibCommon >  List_int_Builder;

common::ComponentBuilder < List<Real>, Component, LibCommon > List_Real_Builder;

common::ComponentBuilder < List<std::string>, Component, LibCommon > List_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const List<bool>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const List<Uint>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const List<int>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const List<Real>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const List<std::string>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
