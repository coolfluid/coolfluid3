#ifndef CF_Mesh_uTests_difference_hpp
#define CF_Mesh_uTests_difference_hpp

// Adapted from K-3D by Bart Janssens
// Copyright (c) 1995-2010, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/math/special_functions/next.hpp>
#include <boost/math/tools/test.hpp>
#include <boost/static_assert.hpp>

#include "Common/CF.hpp"

namespace CF {
namespace Tools {
namespace Difference {

/// Stores the results of the difference::test() function.
class Accumulator
{
public:
  /// Stores statistics for comparisons of exact (string and integer) types, including the number of tests, and the min and max test values.
  boost::accumulators::accumulator_set<bool, boost::accumulators::stats<boost::accumulators::tag::count, boost::accumulators::tag::min, boost::accumulators::tag::max> > exact;
  /// Stores statistics for comparisons of inexact (floating-point) types using Units in the Last Place (ULPS), including the number of tests, min, max, mean, median, and variance.
  boost::accumulators::accumulator_set<Real, boost::accumulators::stats<boost::accumulators::tag::min, boost::accumulators::tag::mean, boost::accumulators::tag::max, boost::accumulators::tag::median, boost::accumulators::tag::lazy_variance> > ulps;
};

/// Function that tests the difference between two objects, returning separate results for exact (integer and string) and inexact (floating-point) types.
/// See "Comparing floating point numbers" by Bruce Dawson at http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
template<typename T>
const Accumulator test(const T& A, const T& B)
{
  Accumulator result;
  test(A, B, result);
  return result;
};

/// Function that tests the difference between two objects, returning separate results for exact (integer and string) and inexact (floating-point) types.
/// See "Comparing floating point numbers" by Bruce Dawson at http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
template<typename T>
void test(const T& A, const T& B, Accumulator& Result)
{
  // This will be triggered if this template is ever instantiated
  BOOST_STATIC_ASSERT(sizeof(T) == 0);
};

/// Specialization of test() that tests bool
inline void test(const bool& A, const bool& B, Accumulator& Result)
{
  Result.exact(A == B);
};

/// Specialization of test that tests int
inline void test(const int& A, const int& B, Accumulator& Result)
{
  Result.exact(A == B);
};

/// Specialization of test that tests char
inline void test(const char& A, const char& B, Accumulator& Result)
{
  Result.exact(A == B);
};

/// Specialization of test that tests Uint
inline void test(const Uint& A, const Uint& B, Accumulator& Result)
{
  Result.exact(A == B);
};

/// Specialization of test that tests Real
inline void test(const Real& A, const Real& B, Accumulator& Result)
{
  Result.ulps(std::fabs(boost::math::float_distance(A, B)));
};

/// Given iterators designating two sequences, calls the test() function for each pair of values,
/// and confirms that both sequences are the same length.
template<typename IteratorT>
void range_test(IteratorT A, IteratorT LastA, IteratorT B, IteratorT LastB, Accumulator& Result)
{
  for(; A != LastA && B != LastB; ++A, ++B)
    test(*A, *B, Result);

  Result.exact(A == LastA && B == LastB);
};

/// Compares vector-like sequences
template<typename VectorT>
void vector_test(const VectorT& A, const VectorT& B, Accumulator& Result)
{
  const Uint sizeA = A.size();
  const Uint sizeB = B.size();
  for(Uint i = 0, j = 0; i != sizeA && j != sizeB; ++i, ++j)
    test(A[i], B[j], Result);

  Result.exact(sizeA == sizeB);
};

} // namespace Difference
} // namespace Tools
} // namespace CF

#endif // !CF_Mesh_uTests_difference_hpp

