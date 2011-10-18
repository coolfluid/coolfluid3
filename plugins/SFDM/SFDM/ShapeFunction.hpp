// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_ShapeFunction_hpp
#define CF_SFDM_ShapeFunction_hpp

#include "Common/BoostArray.hpp"

#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/ShapeFunctionBase.hpp"
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

  typedef boost::shared_ptr<ShapeFunction>       Ptr;
  typedef boost::shared_ptr<ShapeFunction const> ConstPtr;

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
  const boost::multi_array<Uint,3>& points() const { return m_points; }

  enum FaceInfo{ ORIENTATION=0 , SIDE=1 };
  enum Orientation {NEG=0, POS=1};

  Real face_direction(const Uint face) const { return (face_side(face)==RIGHT ? 1. : -1.); }

  const Uint& face_orientation(const Uint face) const { return m_face_info[face][ORIENTATION]; }

  const Uint& face_side(const Uint face) const { return m_face_info[face][SIDE]; }

  const boost::multi_array<Uint,2>& face_points() const { return m_face_points; }

  const boost::multi_array<Uint,2>& face_nb() const { return m_face_number; }

  virtual const SFDM::ShapeFunction& line() const = 0;

  virtual const SFDM::ShapeFunction& flux_line() const = 0;

  virtual const RealMatrix& local_coordinates() const
  {
    throw Common::NotImplemented(FromHere(),"");
    static const RealMatrix obj(1,1);
    return obj;
  }

  virtual RealRowVector value(const RealVector& local_coordinate) const
  {
    RealRowVector values(nb_nodes());
    compute_value(local_coordinate,values);
    return values;
  }
  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    throw Common::NotImplemented(FromHere(),"");
  }
  virtual RealMatrix gradient(const RealVector& local_coordinate) const
  {
    RealMatrix grad(dimensionality(),nb_nodes());
    compute_gradient(local_coordinate,grad);
    return grad;
  }
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    throw Common::NotImplemented(FromHere(),"");
  }




protected:

  /// lookup table for the points
  boost::multi_array<Uint,3> m_points;

  boost::multi_array<Uint,2> m_face_info;

  boost::multi_array<Uint,2> m_face_points;

  boost::multi_array<Uint,2> m_face_number;

  /// storage for number of lines per orientation
  Uint m_nb_lines_per_orientation;



};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

#endif // CF_SFDM_ShapeFunction_hpp
