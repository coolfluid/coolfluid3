// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_math_Hilbert_hpp
#define cf3_math_Hilbert_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/cstdint.hpp>      // for boost::uint_64_t
#include "math/BoundingBox.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

//////////////////////////////////////////////////////////////////////////////

/// @brief Class to compute a global index given a coordinate, based on the
/// Hilbert Spacefilling Curve.
///
/// This algorithm is based on:
/// - John J. Bartholdi and Paul Goldsman "Vertex-Labeling Algorithms for the Hilbert Spacefilling Curve"
/// It is adapted to return contiguous numbers of the boost::uint64_t type, instead of a Real [0,1]
///
/// Given a bounding box and number of hilbert levels, the bounding box can be divided in
/// 2^(dim*levels) equally spaced cells. A given coordinate falling inside one of these cells, is assigned
/// the 1-dimensional Hilbert-index of this cell. To make sure that 1 coordinate corresponds to only 1
/// Hilbert index, the number of levels have to be increased.
/// In 1D, the levels cannot be higher than 32, if you want the indices to fit in "unsigned int" type of 32bit.
/// In 2D, the levels cannot be higher than 15, if you want the indices to fit in "unsigned int" type of 32bit.
/// In 3D, the levels cannot be higher than 10, if you want the indices to fit in "unsigned int" type of 32bit.
///
///
/// No attempt is made to provide the most efficient algorithm. There exist other open-source
/// libraries with more efficient algorithms, such as libhilbert, but its GPL license
/// is not compatible with the LGPL license.
///
/// @author Willem Deconinck
class Hilbert
{
public:

  /// Constructor
  /// Initializes the hilbert space filling curve with a given "space" and "levels"
  Hilbert(const math::BoundingBox& bounding_box, Uint levels);

  /// Compute the hilbert code for a given point, checks for dimension
  boost::uint64_t operator() (const RealVector& point);

  /// Compute the hilbert code for a given point, checks for dimension
  /// @param [out] relative_tolerance  cell-size of smallest level divided by bounding-box size
  boost::uint64_t operator() (const RealVector& point, Real& relative_tolerance);

  /// Compute the hilbert code for a given point in 1D
  boost::uint64_t operator() (const RealVector1& point);

  /// Compute the hilbert code for a given point in 1D
  /// @param [out] relative_tolerance  cell-size of smallest level divided by bounding-box size
  boost::uint64_t operator() (const RealVector1& point, Real& relative_tolerance);

  /// Compute the hilbert code for a given point in 2D
  boost::uint64_t operator() (const RealVector2& point);

  /// Compute the hilbert code for a given point in 2D
  /// @param [out] relative_tolerance  cell-size of smallest level divided by bounding-box size
  boost::uint64_t operator() (const RealVector2& point, Real& relative_tolerance);

  /// Compute the hilbert code for a given point in 3D
  boost::uint64_t operator() (const RealVector3& point);

  /// Compute the hilbert code for a given point in 3D
  /// @param [out] relative_tolerance  cell-size of smallest level divided by bounding-box size
  boost::uint64_t operator() (const RealVector3& point, Real& relative_tolerance);

  /// Return the maximum hilbert code possible with the initialized levels
  ///
  /// Care has to be taken that this number is not larger than the precision of the type storing
  /// the hilbert codes.
  boost::uint64_t max_key() const;

private: // functions

  /// @brief Recursive 1D algorithm
  void recursive_algorithm_1d(const Real& p);

  /// @brief Recursive 2D algorithm
  void recursive_algorithm_2d(const RealVector2& p);

  /// @brief Recursive 3D algorithm
  void recursive_algorithm_3d(const RealVector3& p);

private: // data

  /// Vertex label type (8 vertices in 3D)
  enum VertexLabel {A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7};

  /// maximum recursion level of the Hilbert space filling curve
  Uint m_max_level;

  /// maximum number of unique codes, computed by max_level
  boost::uint64_t m_nb_keys;

  /// Bounding box, defining the space to be filled
  const math::BoundingBox& m_bounding_box;

  /// Dimension
  Uint m_dim;

  /// Current recursion level
  Uint m_level;

  /// Current key at recursion level
  boost::uint64_t m_key;

  /// @name Recursion allocation
  //@{

  /// Quadrant the coordinate lies in. (2D)
  Uint m_quadrant;

  /// Octant the coordinate lies in. (3D)
  Uint m_octant;

  /// Distance between coordinate and a vertex
  Real m_distance;

  /// Minimum distance between coordinate and a vertex
  Real m_min_distance;

  /// index
  Uint m_idx;

  /// box at recursion level. (1D)
  std::vector<Real> m_box1;

  /// box at recursion level. (2D)
  std::vector<RealVector2,Eigen::aligned_allocator<RealVector2> > m_box2;

  /// temporary box at recursion level. (2D)
  std::vector<RealVector2,Eigen::aligned_allocator<RealVector2> > m_tmp_box2;

  /// box at recursion level. (2D)
  std::vector<RealVector3,Eigen::aligned_allocator<RealVector3> > m_box3;

  /// temporary box at recursion level. (3D)
  std::vector<RealVector3,Eigen::aligned_allocator<RealVector3> > m_tmp_box3;

  //@}
};

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_math_Hilbert_hpp
