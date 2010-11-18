// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Triag2DLagrangeP2B_hpp
#define CF_Mesh_SF_Triag2DLagrangeP2B_hpp

#include "Common/BasicExceptions.hpp"

#include "Math/MatrixTypes.hpp"
#include "Math/MathConsts.hpp"

#include "Mesh/Triag2D.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

	using namespace Math::MathConsts;
	
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Triag2DLagrangeP2B  : public Triag2D
{

  /// Number of nodes
  static const Uint nb_nodes = 7;

  /// Order of the shape function
  static const Uint order = 2;

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimension, 1> CoordsT;
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension> NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunctionsT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> MappedGradientT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;
  
  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param map_coord The mapped coordinates
  /// @param shapef Vector storing the result
  static void shape_function(const MappedCoordsT& map_coord, ShapeFunctionsT& shapef)
  {
    const Real L0 = 1.0 - map_coord[0] - map_coord[1];
    const Real L1 = map_coord[0];
    const Real L2 = map_coord[1];

    const Real Phi6 = L0 * L1 * L2;

    shapef[0] = ( 2*L0 - 1.0 ) * L0 + 3 * Phi6 ;
    shapef[1] = ( 2*L1 - 1.0 ) * L1 + 3 * Phi6 ;
    shapef[2] = ( 2*L2 - 1.0 ) * L2 + 3 * Phi6 ;
    shapef[3] = 4*L0*L1 - 12. * Phi6 ;
    shapef[4] = 4*L1*L2 - 12. * Phi6 ;
    shapef[5] = 4*L2*L0 - 12. * Phi6 ;
    shapef[6] = 27 * Phi6;

  }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param map_coord Store the output mapped coordinates
  template<typename NodesT>
  static void mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& map_coord)
  {
    throw Common::NotImplemented( FromHere(), "" );
    
    const Real invDet = 1. / jacobian_determinant(nodes);

    map_coord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
    map_coord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));

  }

  /// Compute the gradient with respect to mapped coordinates, i.e.
  /// partial derivatives are in terms of the mapped coordinates.
  /// The result needs to be multiplied with the inverse jacobian to get the result in real coordinates.
  /// @param map_coord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void mapped_gradient(const MappedCoordsT& map_coord, MappedGradientT& result)
  {
    const Real L0 = 1.0 - map_coord[0] - map_coord[1];
    const Real L1 = map_coord[0];
    const Real L2 = map_coord[1];

    const Real L0L1 = L0*L1;
    const Real L1L2 = L1*L2;
    const Real L2L0 = L2*L0;

    const Real L2L0_L1L2 = L2L0 - L1L2;
    const Real L0L1_L2L1 = L0L1 - L1L2;

    result(XX, 0) = - (4*L0-1)  +  3*(L2L0_L1L2);
    result(YY, 0) = - (4*L0-1)  +  3*(L0L1_L2L1);

    result(XX, 1) =   (4*L1-1)  +  3*(L2L0_L1L2);
    result(YY, 1) =                3*(L0L1_L2L1);

    result(XX, 2) =                3*(L2L0_L1L2);
    result(YY, 2) =   (4*L2-1)  +  3*(L0L1_L2L1);

    result(XX, 3) =   4*(L0-L1) - 12*(L2L0_L1L2);
    result(YY, 3) = - 4*L1      - 12*(L0L1_L2L1);

    result(XX, 4) =   4*L2      - 12*(L2L0_L1L2);
    result(YY, 4) =   4*L1      - 12*(L0L1_L2L1);

    result(XX, 5) = - 4*L2      - 12*(L2L0_L1L2);
    result(YY, 5) =   4*(L0-L2) - 12*(L0L1_L2L1);

    result(XX, 6) =   27*L2L0_L1L2 ;
    result(YY, 6) =   27*L0L1_L2L1 ;

  }

  /// Compute the jacobian determinant at the given mapped coordinates
  template<typename NodesT>
  static Real jacobian_determinant(const MappedCoordsT& map_coord, const NodesT& nodes)
  {
    return jacobian_determinant(nodes);
  }

  /// Compute the Jacobian matrix
  /// @param map_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void jacobian(const MappedCoordsT& map_coord, const NodesT& nodes, JacobianT& result)
  {
    result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
    result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
    result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
    result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
  }

  /// Compute the adjoint of Jacobian matrix
  /// @param map_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting adjoint
  template<typename NodesT>
  static void jacobian_adjoint(const MappedCoordsT& map_coord, const NodesT& nodes, JacobianT& result)
  {
    throw Common::NotImplemented( FromHere(), "" );

  }

  /// Volume of the cell
  template<typename NodesT>
  static Real volume(const NodesT& nodes)
  {
    throw Common::NotImplemented( FromHere(), "" );
  }
	
  template<typename NodesT>
  static bool in_element(const CoordsT& coord, const NodesT& nodes)
  {
    throw Common::NotImplemented( FromHere(), "" );

    MappedCoordsT mapped_coord;
    mapped_coordinates(coord, nodes, mapped_coord);
    if( (mapped_coord[KSI] >= -eps()) &&
        (mapped_coord[ETA] >= -eps()) &&
        (mapped_coord.sum() <= 1.))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  static const FaceConnectivity& faces();

  Triag2DLagrangeP2B();

  virtual std::string getElementTypeName() const;
  virtual Real computeVolume(const NodesT& coord) const;
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:

/// Helper function for reuse in volume() and jacobian_determinant()
template<typename NodesT>
static Real jacobian_determinant(const NodesT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );

  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);

  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Triag2DLagrangeP2B */
