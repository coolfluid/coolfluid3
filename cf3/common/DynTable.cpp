// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/StreamHelpers.hpp"

#include "common/LibCommon.hpp"
#include "common/DynTable.hpp"

namespace cf3 {
namespace common {

common::ComponentBuilder < DynTable<bool>, Component, LibCommon > DynTable_bool_Builder;

common::ComponentBuilder < DynTable<Uint>, Component, LibCommon > DynTable_Uint_Builder;

common::ComponentBuilder < DynTable<int>, Component, LibCommon >  DynTable_int_Builder;

common::ComponentBuilder < DynTable<Real>, Component, LibCommon > DynTable_Real_Builder;

common::ComponentBuilder < DynTable<std::string>, Component, LibCommon > DynTable_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, DynTable<bool>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, DynTable<Uint>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, DynTable<int>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, DynTable<Real>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, DynTable<std::string>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const DynTable<bool>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(DynTable<bool>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    if (row.size() == 0)
      os << "~";
    else
    {
      boost_foreach(const bool entry, row)
      os << entry << " ";
    }
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const DynTable<Uint>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(DynTable<Uint>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    if (row.size() == 0)
      os << "~";
    else
    {
      boost_foreach(const Uint entry, row)
        os << entry << " ";
    }
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const DynTable<int>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(DynTable<int>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    if (row.size() == 0)
      os << "~";
    else
    {
      boost_foreach(const int entry, row)
        os << entry << " ";
    }
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const DynTable<Real>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(DynTable<Real>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    if (row.size() == 0)
      os << "~";
    else
    {
      boost_foreach(const Real& entry, row)
        os << entry << " ";
    }
    os << "\n";
    ++i;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const DynTable<std::string>& table)
{
  if (table.size())
    os << "\n";
  Uint i=0;
  boost_foreach(DynTable<std::string>::ConstRow row, table.array())
  {
    os << "  " << i << ":  ";
    if (row.size() == 0)
      os << "~";
    else
    {
      boost_foreach(const std::string& entry, row)
        os << entry << " ";
    }
    os << "\n";
    ++i;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
