// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/StreamHelpers.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CTable<bool>, Component, LibMesh > CTable_bool_Builder;

Common::ComponentBuilder < CTable<Uint>, Component, LibMesh > CTable_Uint_Builder;

Common::ComponentBuilder < CTable<int>, Component, LibMesh >  CTable_int_Builder;

Common::ComponentBuilder < CTable<Real>, Component, LibMesh > CTable_Real_Builder;

Common::ComponentBuilder < CTable<std::string>, Component, LibMesh > CTable_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CTable<bool>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Uint>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<int>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Real>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<std::string>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CTable<bool>& table)
{
  if (table.size())
    os << "\n";
  index_foreach(i,CTable<bool>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const bool entry, row)
      os << entry << " ";
    os << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Uint>& table)
{
  if (table.size())
    os << "\n";
  index_foreach(i,CTable<Uint>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Uint entry, row)
      os << entry << " ";
    os << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<int>& table)
{
  if (table.size())
    os << "\n";
  index_foreach(i,CTable<int>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const int entry, row)
      os << entry << " ";
    os << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Real>& table)
{
  if (table.size())
    os << "\n";
  index_foreach(i,CTable<Real>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Real& entry, row)
      os << entry << " ";
    os << "\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<std::string>& table)
{
  if (table.size())
    os << "\n";
  index_foreach(i,CTable<std::string>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const std::string& entry, row)
      os << entry << " ";
    os << "\n";
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
