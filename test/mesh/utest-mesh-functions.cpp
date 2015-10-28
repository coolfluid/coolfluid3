// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::BoundingBox"

#include "common/BoostAssign.hpp"
#include <boost/test/included/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/Functions.hpp"
#include "mesh/LagrangeP1/Quad.hpp"
#include "mesh/LagrangeP2/Quad.hpp"

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( MeshFunctionsTestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( TestNearestNodeMapping )
{
  std::vector<Uint> node_mapping;
  std::vector<bool> is_interior;

  mesh::nearest_node_mapping(mesh::LagrangeP1::Quad::local_coordinates(), mesh::LagrangeP2::Quad::local_coordinates(), node_mapping, is_interior);

  const std::vector<Uint> expected_mapping = boost::assign::list_of(0)(1)(2)(3)(0)(1)(2)(3)(0);
  const Uint nb_target_points = mesh::LagrangeP2::Quad::nb_nodes;
  for(Uint point_idx = 0; point_idx != nb_target_points; ++point_idx)
  {
    BOOST_CHECK_EQUAL(node_mapping[point_idx], expected_mapping[point_idx]);
    BOOST_CHECK(!(is_interior[point_idx] && point_idx != 8));
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

