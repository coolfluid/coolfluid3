// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/StreamHelpers.hpp"
#include "common/Foreach.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/CTable.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < CTable<bool>, Component, LibMesh > CTable_bool_Builder;

common::ComponentBuilder < CTable<Uint>, Component, LibMesh > CTable_Uint_Builder;

common::ComponentBuilder < CTable<int>, Component, LibMesh >  CTable_int_Builder;

common::ComponentBuilder < CTable<Real>, Component, LibMesh > CTable_Real_Builder;

common::ComponentBuilder < CTable<std::string>, Component, LibMesh > CTable_string_Builder;

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
  Uint i=0;
  boost_foreach(CTable<bool>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const bool entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Uint>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(CTable<Uint>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Uint entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<int>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(CTable<int>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const int entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<Real>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(CTable<Real>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Real& entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const CTable<std::string>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(CTable<std::string>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const std::string& entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
