// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Proposition for new element / shapefunction API"

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Component.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/ElementType.hpp"
#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP0/Triag.hpp"
#include "mesh/LagrangeP1/Line.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"

#include "mesh/ElementTypes.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

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

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Test_ShapeFunction_TestSuite, Test_ShapeFunction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( sf_static_version )
{
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

  boost::shared_ptr<ShapeFunction> sf = allocate_component< ShapeFunctionT<LagrangeP0::Triag> >("sf");

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

  boost::shared_ptr< ElementType > etype = build_component_abstract_type<ElementType>("cf3.mesh.LagrangeP1.Triag2D","etype");
  etype->compute_centroid(nodes,centroid);
  std::cout << "dynamic: centroid = " << centroid.transpose() << std::endl;

  // Check if compute_normal throws, as it is not implemented in the static implementation
  Core::instance().environment().options().set("exception_outputs",false);
  Core::instance().environment().options().set("exception_backtrace",false);
  BOOST_CHECK_THROW(etype->compute_normal(nodes,centroid),common::NotImplemented);
  Core::instance().environment().options().set("exception_outputs",true);
  Core::instance().environment().options().set("exception_backtrace",true);

  boost::shared_ptr< ElementType > quad_face = build_component_abstract_type<ElementType>("cf3.mesh.LagrangeP1.Quad3D","etype");

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


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

