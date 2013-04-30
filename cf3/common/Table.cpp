// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/StreamHelpers.hpp"
#include "common/Foreach.hpp"

#include "common/LibCommon.hpp"
#include "common/Table.hpp"

namespace cf3 {
namespace common {

common::ComponentBuilder < Table<bool>, Component, LibCommon > Table_bool_Builder;

common::ComponentBuilder < Table<Uint>, Component, LibCommon > Table_Uint_Builder;

common::ComponentBuilder < Table<int>, Component, LibCommon >  Table_int_Builder;

common::ComponentBuilder < Table<Real>, Component, LibCommon > Table_Real_Builder;

common::ComponentBuilder < Table<std::string>, Component, LibCommon > Table_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Table<bool>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<Uint>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<int>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<Real>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<std::string>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Table<bool>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(Table<bool>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const bool entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<Uint>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(Table<Uint>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Uint entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<int>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(Table<int>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const int entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<Real>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(Table<Real>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    boost_foreach(const Real& entry, row)
      os << entry << " ";
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Table<std::string>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(Table<std::string>::ConstRow row, table.array())
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

} // common
} // cf3
