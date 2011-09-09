// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Proposition for new element / shapefunction API"

#include <boost/test/unit_test.hpp>
#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/Core.hpp"
#include "Common/CEnv.hpp"

#include "Mesh/ElementType.hpp"
#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/LagrangeP0/Triag.hpp"
#include "Mesh/LagrangeP1/Line.hpp"
#include "Mesh/LagrangeP1/Triag2D.hpp"

#include "Mesh/ElementTypes.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

struct Test_ShapeFunction_Fixture
{
  /// common setup for each test case
  Test_ShapeFunction_Fixture()
  {
  }

  /// common tear-down for each test case
  ~Test_ShapeFunction_Fixture()
  {
  }
};


template <typename SF, typename TR>
class FallBack
{
public:

  enum { nb_nodes       = TR::nb_nodes };
  enum { dimensionality = TR::dimensionality };
  enum { order          = TR::order };
  static const GeoShape::Type shape = TR::shape;
  typedef Eigen::Matrix<Real, dimensionality, 1       >  MappedCoordsT;
  typedef Eigen::Matrix<Real, 1             , nb_nodes>  ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes>  GradientT;

  static const RealMatrix& local_coordinates()
  {
    throw Common::NotImplemented(FromHere(),"");
    const static RealMatrix real_matrix_obj;
    return real_matrix_obj;
  }

  static ValueT value(const MappedCoordsT& mapped_coord)
  {
    throw Common::NotImplemented(FromHere(),"");
    return ValueT();
  }
};

template <typename SF,typename TR>
const GeoShape::Type FallBack<SF,TR>::shape;

struct ConcreteSF_traits
{
  enum { nb_nodes       = 2              };
  enum { dimensionality = 1              };
  enum { order          = 1              };
  enum { shape          = GeoShape::LINE };
};


struct Mesh_API ConcreteSF : public FallBack<ConcreteSF,ConcreteSF_traits>
{
};


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Test_ShapeFunction_TestSuite, Test_ShapeFunction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( sf_static_version )
{
  std::cout << "nb_nodes = " << ConcreteSF::nb_nodes << std::endl;
  std::cout << "shape = " << ConcreteSF::shape << std::endl;

  LagrangeP0::Triag::MappedCoordsT mapped_coord = (LagrangeP0::Triag::MappedCoordsT() << 0, 0 ).finished();

  LagrangeP0::Triag::ValueT values       = LagrangeP0::Triag::value( mapped_coord );
  const RealMatrix&             local_coords = LagrangeP0::Triag::local_coordinates();

  std::cout << "static : values       = " << values       << std::endl;
  std::cout << "static : local_coords = " << local_coords << std::endl;

  LagrangeP0::Triag::compute_value(mapped_coord, values);
  std::cout << "static : values       = " << values       << std::endl;

  const RealMatrix&            line_local_coords = LagrangeP1::Line::local_coordinates();

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( sf_dynamic_version )
{
  RealVector mapped_coord = (RealVector(2) << 0, 0).finished();

  std::auto_ptr<ShapeFunction> sf (new ShapeFunctionT<LagrangeP0::Triag>);

  RealRowVector     values(1);//       = sf->value(mapped_coord);
  const RealMatrix& local_coords = sf->local_coordinates();

//  std::cout << "dynamic: values       = " << values       << std::endl;
  std::cout << "dynamic: local_coords = " << local_coords << std::endl;

  sf->compute_value(mapped_coord,values);
  std::cout << "dynamic: values       = " << values       << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( etype_static_version )
{
  typedef LagrangeP1::Triag2D  ETYPE;
  ETYPE::NodesT nodes = (ETYPE::NodesT() <<
                         0, 0,
                         1, 0,
                         0, 1
                         ).finished();
  ETYPE::CoordsT centroid;
  ETYPE::compute_centroid(nodes,centroid);
  std::cout << "static : centroid = " << centroid.transpose() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( etype_dynamic_version )
{
  RealMatrix nodes = (RealMatrix(3,2) <<
                      0, 0,
                      1, 0,
                      0, 1
                      ).finished();
  RealVector centroid(2);

  ElementType::Ptr etype = build_component_abstract_type<ElementType>("CF.Mesh.LagrangeP1.Triag2D","etype");
  etype->compute_centroid(nodes,centroid);
  std::cout << "dynamic: centroid = " << centroid.transpose() << std::endl;

  // Check if compute_normal throws, as it is not implemented in the static implementation
  Core::instance().environment().configure_option("exception_outputs",false);
  Core::instance().environment().configure_option("exception_backtrace",false);
  BOOST_CHECK_THROW(etype->compute_normal(nodes,centroid),Common::NotImplemented);
  Core::instance().environment().configure_option("exception_outputs",true);
  Core::instance().environment().configure_option("exception_backtrace",true);

  ElementType::Ptr quad_face = build_component_abstract_type<ElementType>("CF.Mesh.LagrangeP1.Quad3D","etype");

  RealMatrix quad_face_nodes = (RealMatrix(4,3) <<
                      0, 0, 0,
                      1, 0, 0,
                      1, 1, 0,
                      0, 1, 0
                      ).finished();

  RealVector normal(quad_face->dimension());
  quad_face->compute_normal(quad_face_nodes,normal);
  std::cout << "dynamic: normal = " << normal.transpose() << std::endl;
  std::cout << "dynamic: sf = " << etype->shape_function().derived_type_name() << std::endl;
  std::cout << "dynamic: sf = " << quad_face->shape_function().derived_type_name() << std::endl;
  std::cout << "dynamic: sf = " << etype->face_type(0).derived_type_name() << std::endl;

  std::cout << "dynamic: type = " << etype->derived_type_name() << std::endl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( element_types )
{
  struct PrintType
  {
    public: // functions

      /// Operator
      template < typename ETYPE >
      void operator() ( ETYPE& T )
      {
        std::cout << ETYPE::type_name() << std::endl;
      }

  } print_type;

  boost::mpl::for_each< Mesh::ElementTypes >(print_type);
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

