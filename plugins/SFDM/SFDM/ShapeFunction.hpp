// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_ShapeFunction_hpp
#define cf3_SFDM_ShapeFunction_hpp

#include "common/BoostArray.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/ShapeFunction.hpp"
#include "mesh/GeoShape.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace SFDM {

/// @brief Spectral Finite Difference shape function base class
///
/// SFD shape functions are comprised of 1D shape functions, in every direction of the
/// element dimensionality. The total shape function is then the tensorial product of these
/// 1D shape functions.
/// Therefore the only possible SFD element types are Lines (1D), Quadrilaterals(2D), Hexahedrals(3D)
class SFDM_API ShapeFunction  : public mesh::ShapeFunction {
public:

  typedef boost::detail::multi_array::multi_array_view<Real,2> FieldView;


private:

  typedef const boost::detail::multi_array::const_sub_array<Uint,1> RowRef;

public:

  /// Constructor
  ShapeFunction(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "ShapeFunction"; }

  // Concrete implementation
  virtual const RealMatrix& local_coordinates() const { return sol_pts(); }

  // Concrete implementation
  virtual RealRowVector value(const RealVector& local_coordinate) const;

  // Concrete implementation
  virtual RealMatrix gradient(const RealVector& local_coordinate) const;

  // Concrete implementation
  virtual Uint nb_nodes() const { return nb_sol_pts(); }

  /// Access a list of used solution points contributed to by flx_pt for derivative to direction
  virtual const std::vector<Uint>& interpolate_grad_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const = 0;

  /// Access a list of used solution points contributed to by flx_pt for derivative to direction
  virtual const std::vector<Uint>& interpolate_sol_to_flx_used_sol_pts(const Uint flx_pt) const = 0;

  /// Access a list of used solution points contributed to by flx_pt for derivative to direction
  virtual const std::vector<Uint>& interpolate_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const = 0;

  /// Access the coefficient for flx_pt, for the derivative to direction in sol_pt
  virtual const Real& interpolate_grad_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const = 0;

  /// Access the coefficient for flx_pt, for the interpolation contribution from sol_pt
  virtual const Real& interpolate_sol_to_flx_coeff(const Uint flx_pt, const Uint sol_pt) const = 0;

  /// Access the coefficient for interpolation contribution from flx_pt, for the interpolation to sol_pt
  virtual const Real& interpolate_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const = 0;

  /// Number of solution points
  virtual Uint nb_sol_pts() const = 0;

  /// Number of flux points
  virtual Uint nb_flx_pts() const = 0;

  /// Solution point coordinates (rows are coordinates)
  virtual const RealMatrix& sol_pts() const = 0;

  /// Flux point coordinates (rows are coordinates)
  virtual const RealMatrix& flx_pts() const = 0;

  /// Directions flux point contributes to
  virtual const std::vector<Uint>& flx_pt_dirs(const Uint flx_pt) const = 0;

  /// List of interior flux points
  virtual const std::vector<Uint>& interior_flx_pts() const = 0;

  /// List of flux points in a given face
  virtual const std::vector<Uint>& face_flx_pts(const Uint face_idx) const = 0;

  /// Sign to be multiplied with the flux computed in flx_pt
  virtual const Real& flx_pt_sign(const Uint flx_pt, const Uint dir) const = 0;

};

////////////////////////////////////////////////////////////////////////////////

inline RealRowVector ShapeFunction::value(const RealVector& local_coordinate) const
{
  RealRowVector values(nb_sol_pts());
  compute_value(local_coordinate,values);
  return values;
}

// Concrete implementation
inline RealMatrix ShapeFunction::gradient(const RealVector& local_coordinate) const
{
  RealMatrix grad(dimensionality(),nb_sol_pts());
  compute_gradient(local_coordinate,grad);
  return grad;
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

#endif // cf3_SFDM_ShapeFunction_hpp
