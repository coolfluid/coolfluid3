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

#ifndef cf3_mesh_ElementTypeBase_hpp
#define cf3_mesh_ElementTypeBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include "common/StringConversion.hpp"
#include "math/MatrixTypes.hpp"
#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  class  ElementType;
  struct ElementTypeFaceConnectivity;

////////////////////////////////////////////////////////////////////////////////

/// @brief Fallback class if a concrete Element Type doesn't implement a static function
///
/// Implements all functions ElementTypeT expects.
/// When creating a new element type, this list shows all the possible static functions
/// that can be implemented and are recognized by the dynamic interface from ElementType
/// @author Willem Deconinck
template <typename ETYPE, typename TR>
class ElementTypeBase
{
public:

  // Information coming from Traits
  // ------------------------------
  typedef typename TR::SF SF;
  static const GeoShape::Type shape = (GeoShape::Type) TR::SF::shape;
  static const Uint order          = (Uint) TR::SF::order          ;
  static const Uint dimensionality = (Uint) TR::SF::dimensionality ;
  static const Uint dimension      = (Uint) TR::dimension          ;
  static const Uint nb_faces       = (Uint) TR::nb_faces           ;
  static const Uint nb_edges       = (Uint) TR::nb_edges           ;
  static const Uint nb_nodes       = (Uint) TR::SF::nb_nodes       ;

  static std::string type_name() { return GeoShape::Convert::instance().to_str(shape)+common::to_str((Uint)dimension)+"D"; }

  // Typedefs for special matrices
  // -----------------------------
  typedef typename TR::SF::MappedCoordsT                 MappedCoordsT;
  typedef Eigen::Matrix<Real, TR::dimension, 1>              CoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, TR::dimension>       NodesT;
  typedef Eigen::Matrix<Real, TR::SF::dimensionality, TR::dimension> JacobianT;

  // Not-implemented static functions
  // --------------------------------
  static void compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord);
  static Real jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes);


  template < typename MatrixType >
  static void compute_jacobian( const MappedCoordsT& mapped_coord, const NodesT& nodes, MatrixType& jacobian )
  {
    throw_not_implemented(FromHere());
  }

  template < typename MatrixType >
  static typename boost::enable_if< boost::is_same< MatrixType, JacobianT > >::type
  compute_jacobian_if_enabled( const MappedCoordsT& mapped_coord, const NodesT& nodes, MatrixType& jacobian )
  {
    ETYPE::compute_jacobian( mapped_coord, nodes, jacobian );
  }

  template < typename MatrixType >
  static typename boost::enable_if< boost::mpl::not_<boost::is_same< MatrixType, JacobianT > > >::type
  compute_jacobian_if_enabled( const MappedCoordsT& mapped_coord, const NodesT& nodes, MatrixType& jacobian )
  {
    throw common::ShouldNotBeHere(FromHere(), "Jacobian matrix dimensions are wrong!");
  }


  static void compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_normal(const NodesT& nodes, CoordsT& normal);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);
  static void compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result);

  // Implemented static functions
  // ----------------------------
  static MappedCoordsT mapped_coordinate(const CoordsT& coord, const NodesT& nodes);
  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static CoordsT plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation);

private:

  static void throw_not_implemented(const common::CodeLocation& where);

};

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
const GeoShape::Type ElementTypeBase<ETYPE,TR>::shape;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::order;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::dimensionality;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::dimension;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::nb_faces;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::nb_edges;

template <typename ETYPE,typename TR>
const Uint ElementTypeBase<ETYPE,TR>::nb_nodes;

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
inline void ElementTypeBase<ETYPE,TR>::throw_not_implemented(const common::CodeLocation& where)
{
  throw common::NotImplemented(where,"static function not implemented / not applicable for element type ["+ETYPE::type_name()+"]");
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
typename ElementTypeBase<ETYPE,TR>::MappedCoordsT ElementTypeBase<ETYPE,TR>::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return MappedCoordsT();
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
void ElementTypeBase<ETYPE,TR>::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
Real ElementTypeBase<ETYPE,TR>::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
typename ElementTypeBase<ETYPE,TR>::JacobianT ElementTypeBase<ETYPE,TR>::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return JacobianT();
}

////////////////////////////////////////////////////////////////////////////////

//template <typename ETYPE,typename TR>
//void ElementTypeBase<ETYPE,TR>::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, Eigen::Matrix<Real,0,1>& jacobian)
//{
//  throw_not_implemented(FromHere());
//}



////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
void ElementTypeBase<ETYPE,TR>::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
Real ElementTypeBase<ETYPE,TR>::volume(const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
Real ElementTypeBase<ETYPE,TR>::area(const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
void ElementTypeBase<ETYPE,TR>::compute_normal(const NodesT& nodes, CoordsT& normal)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
void ElementTypeBase<ETYPE,TR>::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
bool ElementTypeBase<ETYPE,TR>::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  throw_not_implemented(FromHere());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
typename ElementTypeBase<ETYPE,TR>::CoordsT ElementTypeBase<ETYPE,TR>::plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation)
{
  throw_not_implemented(FromHere());
  return CoordsT();
}

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE,typename TR>
void ElementTypeBase<ETYPE,TR>::compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result)
{
  throw_not_implemented(FromHere());
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementTypeBase_hpp
