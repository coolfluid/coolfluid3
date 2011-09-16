// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_ShapeFunction_hpp
#define CF_SFDM_ShapeFunction_hpp

#include "Common/BoostArray.hpp"

#include "Mesh/ShapeFunction.hpp"
#include "Math/Defs.hpp"

#include "SFDM/LibSFDM.hpp"

namespace CF {
namespace SFDM {

/// @brief Spectral Finite Difference shape function base class
///
/// SFD shape functions are comprised of 1D shape functions, in every direction of the
/// element dimensionality. The total shape function is then the tensorial product of these
/// 1D shape functions.
/// Therefore the only possible SFD element types are Lines (1D), Quadrilaterals(2D), Hexahedrals(3D)
class SFDM_API ShapeFunction  : public Mesh::ShapeFunction {
public:

  /// 3 dimensional array of Uint (orientation,line,points)
  typedef const boost::multi_array<Uint,3>& Points;

  /// 2 dimensional array of Uint (line,points). view of Points
  typedef const boost::const_subarray_gen< boost::multi_array<Uint,3> ,2>::type Lines;

  /// 1 dimensional array of Uint (points). view of Lines
  typedef const boost::const_subarray_gen< boost::multi_array<Uint,3> ,1>::type LinePoints;

  /// Constructor
  ShapeFunction(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "ShapeFunction"; }

  /// @brief Table to get point indexes given an orientation, a line, a point on the line
  ///
  /// Get the index corresponding to an orientation, a line, and a index on that line
  /// points()[orientation][line_idx][line_point_idx]
  /// - orientation     orientation of the line (KSI / ETA / ZTA)
  /// - line_idx        index of the line following the given orientation
  /// - line_point_idx  index inside the line
  ///
  /// Lines      points()[orientation]           for a view of the orientation
  /// LinePoints points()[orientation][line_idx] for a view of one line
  Points points() const { return m_points; }

  /// @brief Table to get face point indexes given an orientation, a line, a side
  ///
  /// Get the index corresponding to an orientation, a line, and a index on that line
  /// points()[orientation][line_idx][side]
  /// - orientation  orientation of the line (KSI / ETA / ZTA)
  /// - line_idx     index of the line following the given orientation
  /// - side         left or right side of the line (LEFT / RIGHT)
  ///
  /// Lines      face_points()[orientation]           for a view of the orientation
  /// LinePoints face_points()[orientation][line_idx] for a view of one line
  Points face_points() const { return m_face_points; }

  const boost::multi_array<Uint,2>& face_number() const { return m_face_number; }

  /// Number of lines per orientation
  /// @returns number of lines per orientation
  Uint nb_lines_per_orientation() const { return m_nb_lines_per_orientation; }

  /// Number of nodes per line
  /// @returns number of nodes per line
  Uint nb_nodes_per_line() const { return order()+1; }

  virtual const ShapeFunction& line() const;

protected:

  /// lookup table for the points
  boost::multi_array<Uint,3> m_points;

  /// lookup table for the face_points
  boost::multi_array<Uint,3> m_face_points;

  /// lookup table for the face_number
  boost::multi_array<Uint,2> m_face_number;

  /// storage for number of lines per orientation
  Uint m_nb_lines_per_orientation;

};

} // SFDM
} // CF

#endif // CF_SFDM_ShapeFunction_hpp
