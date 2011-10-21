// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/CList.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

//////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CList<bool>, Component, LibMesh > CList_bool_Builder;

common::ComponentBuilder < CList<Uint>, Component, LibMesh > CList_Uint_Builder;

common::ComponentBuilder < CList<int>, Component, LibMesh >  CList_int_Builder;

common::ComponentBuilder < CList<Real>, Component, LibMesh > CList_Real_Builder;

common::ComponentBuilder < CList<std::string>, Component, LibMesh > CList_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CList<bool>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CList<Uint>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CList<int>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CList<Real>& list)
{
  if (list.size())
    os << "\n";
  for (Uint i=0; i<list.size(); ++i)
  {
    os << "  " << i << ":  " << list[i] << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CList<std::string>& list)
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

} // mesh
} // cf3
