// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_Rotate_hpp
#define cf3_mesh_actions_Rotate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that rotates the mesh around a rotation point in 2D
/// and around an axis in 3D
/// @author Willem Deconinck
class mesh_actions_API Rotate : public MeshTransformer
{   
public: // functions
  
  /// constructor
  Rotate( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "Rotate"; }

  virtual void execute();
  
private: // functions

  /// @brief Compute transformation matrix for a 2d rotation around centre point
  /// @param [in]  rotation_centre   Centre of rotation
  /// @param [in]  theta             Angle of rotation in radians
  /// @return transformation matrix (2x3), the 3rd column being a translation vector.
  ///         P' = M(:,1:2)*P + M(:,3)   in matlab notation
  Eigen::Matrix<Real,2,3> compute_rotation_matrix_2d(const RealVector2& rotation_centre, const Real& theta);
  
  /// @brief Compute transformation matrix for a 2d rotation around centre point
  /// @param [in]  point_on_axis   Any point on the rotation axis
  /// @param [in]  axis_direction  Vector describing the directioni of the rotation axis
  /// @param [in]  theta           Angle of rotation in radians, around rotation axis
  /// @return transformation matrix (3x4), the 4th column being a translation vector.
  ///         P' = M(:,1:3)*P + M(:,4)   in matlab notation
  Eigen::Matrix<Real,3,4> compute_rotation_matrix_3d(const RealVector3& point_on_axis, const RealVector3& axis_direction, const Real& theta);
  
  template <typename vector_t>
  void rotate_point_2d(const Eigen::Matrix<Real,2,3>& matrix, vector_t& point);

  template <typename vector_t>
  void rotate_point_3d(const Eigen::Matrix<Real,3,4>& matrix, vector_t& point);
     
private: // data

  Real x,y,z;   ///< allocations for coordinates
}; // end Rotate

//////////////////////////////////////////////////////////////////////////////

template <typename vector_t>
inline void Rotate::rotate_point_2d(const Eigen::Matrix<Real,2,3>& m, vector_t& p)
{
  enum { XX=0, YY=1, TT=2 };
  x = p[XX];
  y = p[YY];
  p[XX] = m(XX,XX)*x + m(XX,YY)*y + m(XX,TT);
  p[YY] = m(YY,XX)*x + m(YY,YY)*y + m(YY,TT);
}

template <typename vector_t>
inline void Rotate::rotate_point_3d(const Eigen::Matrix<Real,3,4>& m, vector_t& p)
{
  enum { XX=0, YY=1, ZZ=2, TT=3 };
  x = p[XX];
  y = p[YY];
  z = p[ZZ];

  p[XX] = m(XX,XX)*x + m(XX,YY)*y + m(XX,ZZ)*z + m(XX,TT);
  p[YY] = m(YY,XX)*x + m(YY,YY)*y + m(YY,ZZ)*z + m(YY,TT);
  p[ZZ] = m(ZZ,XX)*x + m(ZZ,YY)*y + m(ZZ,ZZ)*z + m(ZZ,TT);
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_Rotate_hpp
