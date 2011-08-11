// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CList.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

//////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CList<bool>, Component, LibMesh > CList_bool_Builder;

Common::ComponentBuilder < CList<Uint>, Component, LibMesh > CList_Uint_Builder;

Common::ComponentBuilder < CList<int>, Component, LibMesh >  CList_int_Builder;

Common::ComponentBuilder < CList<Real>, Component, LibMesh > CList_Real_Builder;

Common::ComponentBuilder < CList<std::string>, Component, LibMesh > CList_string_Builder;

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

} // Mesh
} // CF
