// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief This file deals with the translation from the dynamic API of
/// ElementType to static implementations of element types.
///
/// Implementations of Element types don't inherit from Common::Component
/// e.g. LagrangeP1::Triag2D. \n
/// The actual concrete component is created as ElementTypeT<LagrangeP1::Triag2D>
///
/// @author Willem Deconinck

#ifndef CF_Mesh_ElementTypeT_hpp
#define CF_Mesh_ElementTypeT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// @macro Check if a class has a static member function defined
/// based on SFINAE principle : substitution failure is not an error
/// example:
/// HAS_STATIC_MEMBER_XXX(foo)
/// HAS_STATIC_MEMBER_XXX(bar)
/// struct MyStruct { static vocmakeid foo(); }
/// bool has_foo = has_static_member_foo<MyStruct>::value; // true
/// bool has_bar = has_static_member_bar<MyStruct>::value; // false
#define HAS_STATIC_MEMBER_XXX(XXX)                                       \
template <typename T, typename FallBackT=void>                           \
struct has_static_member_##XXX                                           \
{                                                                        \
  typedef char Yes;                                                      \
  class No { char c[2]; };                                               \
                                                                         \
  /* murk<> helps to let us call test() with an integer argument */      \
  template <int> struct murk { murk(int); };                             \
                                                                         \
  template <typename S> static Yes test (murk< sizeof(&S::XXX) >);       \
  template <typename S> static No  test (...);                           \
                                                                         \
  enum { value = sizeof(Yes) == sizeof(test<T>(0)) };                    \
  typedef typename boost::mpl::if_c< value , T , FallBackT >::type type; \
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Fallback class if a concrete Element Type doesn't implement a static function
///
/// Implements all functions ElementTypeT expects.
/// When creating a new element type, this list shows all the possible static functions
/// that can be implemented and are recognized by the dynamic interface from ElementType
/// @author Willem Deconinck
struct Mesh_API ElementTypeFallBack
{
public:
  typedef ElementTypeFallBack SF;
  typedef RealVector MappedCoordsT;
  typedef RealVector CoordsT;
  typedef RealMatrix NodesT;
  typedef RealMatrix JacobianT;

  static std::string type_name() { return "ElementTypeFallBack"; }
  static MappedCoordsT mapped_coordinate(const CoordsT& coord, const NodesT& nodes);
  static void compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord);
  static Real jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static JacobianT jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes);
  static void compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& jacobian);
  static void compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result);
  static Real volume(const NodesT& nodes);
  static Real area(const NodesT& nodes);
  static void compute_normal(const NodesT& nodes, CoordsT& normal);
  static void compute_centroid(const NodesT& nodes , CoordsT& centroid);
  static bool is_coord_in_element(const CoordsT& coord, const NodesT& nodes);
  static CoordsT plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation);
  static void compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result);
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Translation class to link concrete static implementations to the dynamic API
/// @author Willem Deconinck
template <typename ETYPE>
class ElementTypeT : public ElementType
{
public: // typedefs

  typedef boost::shared_ptr< ElementTypeT >       Ptr;
  typedef boost::shared_ptr< ElementTypeT const > ConstPtr;

  typedef ElementTypeFallBack FallBack;

  // create const static bool has_static_member_xxx<T>::value
  //     where xxx is replaced by the given argument
  HAS_STATIC_MEMBER_XXX(mapped_coordinate)
  HAS_STATIC_MEMBER_XXX(compute_mapped_coordinate)
  HAS_STATIC_MEMBER_XXX(jacobian)
  HAS_STATIC_MEMBER_XXX(compute_jacobian)
  HAS_STATIC_MEMBER_XXX(compute_jacobian_adjoint)
  HAS_STATIC_MEMBER_XXX(jacobian_determinant)
  HAS_STATIC_MEMBER_XXX(volume)
  HAS_STATIC_MEMBER_XXX(area)
  HAS_STATIC_MEMBER_XXX(compute_normal)
  HAS_STATIC_MEMBER_XXX(compute_centroid)
  HAS_STATIC_MEMBER_XXX(is_coord_in_element)
  HAS_STATIC_MEMBER_XXX(plane_jacobian_normal)
  HAS_STATIC_MEMBER_XXX(compute_plane_jacobian_normal)

public: // functions
  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ElementTypeT( const std::string& name = type_name() ) : ElementType(name)
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
    return ETYPE::shape_function();
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
    typedef typename has_static_member_mapped_coordinate<ETYPE,FallBack>::type ET;
    return ET::mapped_coordinate(coord,nodes);
  }

  virtual void compute_mapped_coordinate(const RealVector& coord, const RealMatrix& nodes, RealVector& mapped_coord) const
  {
    typedef typename has_static_member_compute_mapped_coordinate<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::MappedCoordsT& mapped_c( const_cast<RealVector const&>(mapped_coord) );
    ET::compute_mapped_coordinate(coord, nodes, const_cast<typename ET::MappedCoordsT&>(mapped_c));
    mapped_coord = mapped_c;
  }

  virtual Real jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const
  {
    typedef typename has_static_member_jacobian_determinant<ETYPE,FallBack>::type ET;
    return ET::jacobian_determinant(mapped_coord,nodes);
  }

  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian) const
  {
    typedef typename has_static_member_compute_jacobian<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::JacobianT& jacob( const_cast<RealMatrix const&>(jacobian) );
    ET::compute_jacobian(mapped_coord, nodes, const_cast<typename ET::JacobianT&>(jacob));
    jacobian = jacob;
  }

  virtual RealMatrix jacobian(const RealVector& mapped_coord, const RealMatrix& nodes) const
  {
    typedef typename has_static_member_jacobian<ETYPE,FallBack>::type ET;
    return ET::jacobian(mapped_coord,nodes);
  }

  virtual void compute_jacobian_adjoint(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian_adjoint) const
  {
    typedef typename has_static_member_compute_jacobian_adjoint<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::JacobianT& jacob_adj( const_cast<RealMatrix const&>(jacobian_adjoint) );
    ET::compute_jacobian_adjoint(mapped_coord, nodes, const_cast<typename ET::JacobianT&>(jacob_adj));
    jacobian_adjoint = jacob_adj;
  }


  virtual Real volume(const RealMatrix& nodes) const
  {
    typedef typename has_static_member_volume<ETYPE,FallBack>::type ET;
    return ET::volume(nodes);
  }

  virtual Real area(const RealMatrix& nodes) const
  {
    typedef typename has_static_member_area<ETYPE,FallBack>::type ET;
    return ET::area(nodes);
  }

  virtual void compute_normal(const RealMatrix& nodes, RealVector& normal) const
  {
    typedef typename has_static_member_compute_normal<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::CoordsT& n( const_cast<RealVector const&>(normal) );
    ET::compute_normal(nodes, const_cast<typename ET::CoordsT&>(n));
    normal = n;
  }

  virtual void compute_centroid(const RealMatrix& nodes , RealVector& centroid) const
  {
    typedef typename has_static_member_compute_centroid<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::CoordsT& centr( const_cast<RealVector const&>(centroid) );
    ET::compute_centroid(nodes, const_cast<typename ET::CoordsT&>(centr));
    centroid = centr;
  }

  virtual bool is_coord_in_element(const RealVector& coord, const RealMatrix& nodes) const
  {
    typedef typename has_static_member_is_coord_in_element<ETYPE,FallBack>::type ET;
    return ET::is_coord_in_element(coord,nodes);
  }

  virtual RealVector plane_jacobian_normal(const RealVector& mapped_coord,
                                           const RealMatrix& nodes,
                                           const CoordRef orientation) const
  {
    typedef typename has_static_member_plane_jacobian_normal<ETYPE,FallBack>::type ET;
    return ET::plane_jacobian_normal(mapped_coord,nodes,orientation);
  }

  virtual void compute_plane_jacobian_normal(const RealVector& mapped_coord,
                                             const RealMatrix& nodes,
                                             const CoordRef orientation,
                                             RealVector& result) const
  {
    typedef typename has_static_member_plane_jacobian_normal<ETYPE,FallBack>::type ET;
    // This little hack is necessary and is documented on the Eigen website
    // http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
    const typename ET::CoordsT& res( const_cast<RealVector const&>(result) );
    ET::compute_plane_jacobian_normal(mapped_coord,nodes,orientation,const_cast<typename ET::CoordsT&>(res));
    result = res;
  }

  //@}
};

////////////////////////////////////////////////////////////////////////////////

#undef HAS_STATIC_MEMBER_XXX

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementTypeT_hpp
