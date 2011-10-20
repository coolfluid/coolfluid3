// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "MeshDiff.hpp"

#include "common/Log.hpp"
#include "common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CTable.hpp"


using namespace cf3;
using namespace cf3::common;
using namespace cf3::Mesh;
using namespace Tools::Testing;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {

namespace Testing {

/// Compare 2D arrays
void array2d_test(const CTable<Real>::ArrayT& a, const CTable<Real>::ArrayT& b, Accumulator& result, const std::string& context)
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
void array2d_test(const CTable<Uint>::ArrayT& a, const CTable<Uint>::ArrayT& b, Accumulator& result, const std::string& context)
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

/// Compares CElements
void test(const CElements& a, const CElements& b, Accumulator& result)
{
  const CTable<Uint>::ArrayT& table_a = a.node_connectivity().array();
  const CTable<Uint>::ArrayT& table_b = b.node_connectivity().array();

  array2d_test(table_a, table_b, result, "comparing " + a.uri().path() + " and " + b.uri().path());
}

/// Compares Arrays
void test(const CTable<Real>& a, const CTable<Real>& b, Accumulator& result)
{
  const CTable<Real>::ArrayT& array_a = a.array();
  const CTable<Real>::ArrayT& array_b = b.array();

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

bool diff(const Mesh::CMesh& a, const Mesh::CMesh& b, const Uint max_ulps)
{
  Accumulator accumulator;
  accumulator.max_ulps = max_ulps;
  // Compare Array data TODO: filtered out only coords, because the neu reader always adds a global_indices table
  compare_ranges(find_components_recursively_with_name<CTable<Real> >(a, Mesh::Tags::coordinates()), find_components_recursively_with_name<CTable<Real> >(b, Mesh::Tags::coordinates()), accumulator);
  // Compare connectivity
  compare_ranges(find_components_recursively<CElements>(a), find_components_recursively<CElements>(b), accumulator);

  /// @todo change this to a comparison field per field

  return boost::accumulators::min(accumulator.exact) && (boost::accumulators::max(accumulator.ulps) < max_ulps);
}

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////
