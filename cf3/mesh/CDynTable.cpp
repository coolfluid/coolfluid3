// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/StreamHelpers.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/CDynTable.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < CDynTable<bool>, Component, LibMesh > CDynTable_bool_Builder;

common::ComponentBuilder < CDynTable<Uint>, Component, LibMesh > CDynTable_Uint_Builder;

common::ComponentBuilder < CDynTable<int>, Component, LibMesh >  CDynTable_int_Builder;

common::ComponentBuilder < CDynTable<Real>, Component, LibMesh > CDynTable_Real_Builder;

common::ComponentBuilder < CDynTable<std::string>, Component, LibMesh > CDynTable_string_Builder;

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, CDynTable<bool>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, CDynTable<Uint>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, CDynTable<int>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, CDynTable<Real>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

std::ostream& operator<<(std::ostream& os, CDynTable<std::string>::ConstRow row)
{
  print_vector(os, row);
  return os;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CDynTable<bool>& table)
{
	if (table.size())
		os << "\n";
  Uint i=0;
  boost_foreach(CDynTable<bool>::ConstRow row, table.array())
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

std::ostream& operator<<(std::ostream& os, const CDynTable<Uint>& table)
{
	if (table.size())
		os << "\n";
  Uint i=0;
  boost_foreach(CDynTable<Uint>::ConstRow row, table.array())
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

std::ostream& operator<<(std::ostream& os, const CDynTable<int>& table)
{
	if (table.size())
		os << "\n";
  Uint i=0;
  boost_foreach(CDynTable<int>::ConstRow row, table.array())
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

std::ostream& operator<<(std::ostream& os, const CDynTable<Real>& table)
{
	if (table.size())
		os << "\n";
  Uint i=0;
  boost_foreach(CDynTable<Real>::ConstRow row, table.array())
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

std::ostream& operator<<(std::ostream& os, const CDynTable<std::string>& table)
{
	if (table.size())
		os << "\n";
  Uint i=0;
  boost_foreach(CDynTable<std::string>::ConstRow row, table.array())
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

} // mesh
} // cf3
