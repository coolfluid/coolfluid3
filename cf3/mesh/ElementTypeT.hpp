// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief This file deals with the translation from the dynamic API of
/// ElementType to static implementations of element types.
///
/// Implementations of Element types don't inherit from common::Component
/// e.g. LagrangeP1::Triag2D. \n
/// The actual concrete component is created as ElementTypeT<LagrangeP1::Triag2D>
///
/// @author Willem Deconinck

#ifndef cf3_mesh_ElementTypeT_hpp
#define cf3_mesh_ElementTypeT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/ElementType.hpp"
#include "mesh/ShapeFunctionT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Translation class to link concrete static implementations to the dynamic API
/// @author Willem Deconinck
template <typename ETYPE>
class ElementTypeT : public ElementType
{

public: // functions
  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ElementTypeT( const std::string& name = type_name() ) :
    ElementType(name),
    m_sf(create_component< ShapeFunctionT<typename ETYPE::SF> >(ETYPE::SF::type_name()))
  {
    m_shape          = ETYPE::shape;
    m_nb_nodes       = ETYPE::nb_nodes;
    m_dimensionality = ETYPE::dimensionality;
    m_order          = ETYPE::order;
    m_dimension      = ETYPE::dimension;
    m_nb_faces       = ETYPE::nb_faces;
    m_nb_edges       = ETYPE::nb_edges;
  };

  /// Default destructor
  virtual ~ElementTypeT() {}

  /// Type name
  static std::string type_name() { return ETYPE::type_name(); }

  // @}

  /// @name Accessor functions
  //  ------------------------
  //@{
  /// @return the shape function defining this geometric element
  virtual const ShapeFunction& shape_function() const
  {
    return *m_sf;
  }

  virtual const FaceConnectivity& faces() const
  {
    return ETYPE::faces();
  }

  virtual const ElementType& face_type(const Uint face) const
  {
    return ETYPE::face_type(face);
  }

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{
  virtual RealVector mapped_coordinate(const RealVector& coord, const RealMatrix& nodes) const
  {
    return ETYPE::mapped_coordinate(coord,nodes);
  }

  virtual void compute_mapped_coordinate(const RealVector& coord, const RealMatrix& nodes, RealVector& mapped_coord) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::MappedCoordsT& mapped_c( const_cast<RealVector const&>(mapped_coord) );
    ETYPE::compute_mapped_coordinate(coord, nodes, const_cast<typename ETYPE::MappedCoordsT&>(mapped_c));
    mapped_coord = mapped_c;
  }

  virtual Real jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const
  {
    return ETYPE::jacobian_determinant(mapped_coord,nodes);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::JacobianT& jacob( const_cast<RealMatrix const&>(jacobian) );
    ETYPE::compute_jacobian(mapped_coord, nodes, const_cast<typename ETYPE::JacobianT&>(jacob));
    jacobian = jacob;
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,0,1>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,0,2>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,0,3>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,1,1>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,1,2>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,1,3>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,2,2>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,2,3>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, Eigen::Matrix<Real,3,3>& jacobian) const
  {
    ETYPE::compute_jacobian_if_enabled(mapped_coord, nodes, jacobian);
  }

  virtual RealMatrix jacobian(const RealVector& mapped_coord, const RealMatrix& nodes) const
  {
    return ETYPE::jacobian(mapped_coord,nodes);
  }

  virtual void compute_jacobian_adjoint(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian_adjoint) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::JacobianT& jacob_adj( const_cast<RealMatrix const&>(jacobian_adjoint) );
    ETYPE::compute_jacobian_adjoint(mapped_coord, nodes, const_cast<typename ETYPE::JacobianT&>(jacob_adj));
    jacobian_adjoint = jacob_adj;
  }


  virtual Real volume(const RealMatrix& nodes) const
  {
    return ETYPE::volume(nodes);
  }

  virtual Real area(const RealMatrix& nodes) const
  {
    return ETYPE::area(nodes);
  }

  virtual void compute_normal(const RealMatrix& nodes, RealVector& normal) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::CoordsT& n( const_cast<RealVector const&>(normal) );
    ETYPE::compute_normal(nodes, const_cast<typename ETYPE::CoordsT&>(n));
    normal = n;
  }

  virtual void compute_centroid(const RealMatrix& nodes , RealVector& centroid) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::CoordsT& centr( const_cast<RealVector const&>(centroid) );
    ETYPE::compute_centroid(nodes, const_cast<typename ETYPE::CoordsT&>(centr));
    centroid = centr;
  }

  virtual bool is_coord_in_element(const RealVector& coord, const RealMatrix& nodes) const
  {
    return ETYPE::is_coord_in_element(coord,nodes);
  }

  virtual RealVector plane_jacobian_normal(const RealVector& mapped_coord,
                                           const RealMatrix& nodes,
                                           const CoordRef orientation) const
  {
    return ETYPE::plane_jacobian_normal(mapped_coord,nodes,orientation);
  }

  virtual void compute_plane_jacobian_normal(const RealVector& mapped_coord,
                                             const RealMatrix& nodes,
                                             const CoordRef orientation,
                                             RealVector& result) const
  {
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ETYPE::CoordsT& res( const_cast<RealVector const&>(result) );
    ETYPE::compute_plane_jacobian_normal(mapped_coord,nodes,orientation,const_cast<typename ETYPE::CoordsT&>(res));
    result = res;
  }

  //@}

private:
  Handle< ShapeFunction > m_sf;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementTypeT_hpp
