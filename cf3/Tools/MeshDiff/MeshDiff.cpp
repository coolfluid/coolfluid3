// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"


#include "Tools/MeshDiff/MeshDiff.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace Tools::Testing;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {

namespace Testing {

/// Compare 2D arrays
void array2d_test(const Table<Real>::ArrayT& a, const Table<Real>::ArrayT& b, Accumulator& result, const std::string& context)
{
  const Uint size_a = a.size();
  const Uint size_b = b.size();

  for(Uint i = 0, j = 0; i != size_a && j != size_b; ++i, ++j)
  {
    Accumulator acc;
    acc.max_ulps = result.max_ulps;
    vector_test(a[i], b[j], acc);
    const bool exact = boost::accumulators::min(acc.exact);
    const Real ulps = boost::accumulators::max(acc.ulps);
    if(!exact || ulps > acc.max_ulps)
    {
      CFerror << "In " << context << ", row " << i << ":\n";
      BOOST_FOREACH(const Real& val, a[i])
        CFerror << "  " << val;
      CFerror << "\n    differs from\n";
      BOOST_FOREACH(const Real& val, b[i])
        CFerror << "  " << val;
      CFerror << CFendl;
      if(exact)
        CFerror << "  ulps: " << ulps << CFendl;
    }
    result.exact(exact);
    result.ulps(ulps);
  }

  result.exact(size_a == size_b);

  if(size_a != size_b)
    CFerror << "Size difference in " << context << ": " << size_a << " != " << size_b << CFendl;
}

/// Compare 2D arrays
void array2d_test(const Table<Uint>::ArrayT& a, const Table<Uint>::ArrayT& b, Accumulator& result, const std::string& context)
{
  const Uint size_a = a.size();
  const Uint size_b = b.size();

  for(Uint i = 0, j = 0; i != size_a && j != size_b; ++i, ++j)
  {
    Accumulator acc;
    vector_test(a[i], b[j], acc);
    const bool exact = boost::accumulators::min(acc.exact);
    if(!exact)
    {
      CFerror << "In " << context << ", row " << i << ":\n";
      BOOST_FOREACH(const Uint& val, a[i])
        CFerror << "  " << val;
      CFerror << "\n    differs from\n";
      BOOST_FOREACH(const Uint& val, b[i])
        CFerror << "  " << val;
      CFerror << CFendl;
    }
    result.exact(exact);
    result.ulps(0.);
  }

  result.exact(size_a == size_b);

  if(size_a != size_b)
    CFerror << "Size difference in " << context << ": " << size_a << " != " << size_b << CFendl;
}

/// Compares Elements
void test(const Elements& a, const Elements& b, Accumulator& result)
{
  const Table<Uint>::ArrayT& table_a = a.geometry_space().connectivity().array();
  const Table<Uint>::ArrayT& table_b = b.geometry_space().connectivity().array();

  array2d_test(table_a, table_b, result, "comparing " + a.uri().path() + " and " + b.uri().path());
}

/// Compares Arrays
void test(const Table<Real>& a, const Table<Real>& b, Accumulator& result)
{
  const Table<Real>::ArrayT& array_a = a.array();
  const Table<Real>::ArrayT& array_b = b.array();

  array2d_test(array_a, array_b, result, "comparing " + a.uri().path() + " and " + b.uri().path());
}

}

namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

/// Compares two ranges
template<typename RangeT>
void compare_ranges(const RangeT& a, const RangeT& b, Accumulator& accumulator)
{
  range_test(a.begin(), a.end(), b.begin(), b.end(), accumulator);
}

bool diff(const mesh::Mesh& a, const mesh::Mesh& b, const Uint max_ulps)
{
  Accumulator accumulator;
  accumulator.max_ulps = max_ulps;
  // Compare Array data TODO: filtered out only coords, because the neu reader always adds a global_indices table
  compare_ranges(find_components_recursively_with_name<Table<Real> >(a, mesh::Tags::coordinates()), find_components_recursively_with_name<Table<Real> >(b, mesh::Tags::coordinates()), accumulator);
  // Compare connectivity
  compare_ranges(find_components_recursively<Elements>(a), find_components_recursively<Elements>(b), accumulator);

  /// @todo change this to a comparison field per field

  return boost::accumulators::min(accumulator.exact) && (boost::accumulators::max(accumulator.ulps) < max_ulps);
}

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////
