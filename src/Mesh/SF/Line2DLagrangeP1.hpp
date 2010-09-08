#ifndef CF_Mesh_SF_Line2DLagrangeP1_hpp
#define CF_Mesh_SF_Line2DLagrangeP1_hpp

#include "Math/RealMatrix.hpp"

#include "Mesh/Line2D.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// line element that lives in 2D space
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct Line2DLagrangeP1  : public Line2D
{
  Line2DLagrangeP1();

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mappedCoord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function(const RealVector& mappedCoord, RealVector& shapeFunc)
  {
    cf_assert(shapeFunc.size() == nb_nodes);
    cf_assert(mappedCoord.size() == dimensionality);

    shapeFunc[0] = 0.5 * (1.0 - mappedCoord[KSI]);
    shapeFunc[1] = 0.5 * (1.0 + mappedCoord[KSI]);
  }

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates.
  /// @param mappedCoord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void mapped_gradient(const RealVector& mappedCoord, RealMatrix& result)
  {
    cf_assert(result.nbCols() == nb_nodes);
    cf_assert(result.nbRows() == dimension);
    result(XX, 0) = -0.5;
    result(XX, 1) = 0.5;
  }

  /// Compute the Jacobian matrix
  /// In the case of the Line2D element, this is the vector corresponding to the line segment
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void jacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result)
  {
    cf_assert(result.nbRows() == dimensionality);
    cf_assert(result.nbCols() == dimension);
    result(KSI,XX) = 0.5*(nodes[1][XX] - nodes[0][XX]);
    result(KSI,YY) = 0.5*(nodes[1][YY] - nodes[0][YY]);
  }

  /// Normal vector to the surface. Length equals the jacobian norm.
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void normal(const RealVector& mappedCoord, const NodesT& nodes, RealVector& result)
  {
    cf_assert(result.size() == dimension);
    result[XX] = 0.5*( nodes[1][YY] - nodes[0][YY]);
    result[YY] = 0.5*(-nodes[1][XX] + nodes[0][XX]);
  }

  /// Volume of the cell. 0 in case of elements with a dimensionality that is less than
  /// the dimension of the problem
  template<typename NodesT>
  static Real volume(const NodesT& nodes)
  {
    return 0.;
  }

  /// The area of an element that represents a surface in the solution space, i.e.
  /// 1D elements in 2D space or 2D elements in 3D space
  template<typename NodesT>
  static Real area(const NodesT& nodes)
  {
    RealVector v = nodes[1] - nodes[0];
    return v.norm2();
  }

  /// Number of nodes
  static const Uint nb_nodes = 2;

  /// Order of the shape function
  static const Uint order = 1;

  /// Given nodal values, write the interpolation
  template<typename NodalValuesT, typename ValueT>
  void operator()(const RealVector& mapped_coord, const NodalValuesT& nodal_values, ValueT& interpolation) const
  {
    cf_assert(mapped_coord.size() == dimensionality);
    cf_assert(nodal_values.size() == nb_nodes);
    const Real ksi = mapped_coord[KSI];
    interpolation = 0.5 * ((1. - ksi) * nodal_values[0] + (1. + ksi) * nodal_values[1]);
  }

  virtual std::string getElementTypeName() const;

  /// The volume of an element with a dimensionality that is less than
  /// the dimension of the problem is 0.
  virtual Real computeVolume(const NodesT& coord) const;
  
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Line2DLagrangeP1 */
