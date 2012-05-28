// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ETYPE shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "common/Table.hpp"
#include "mesh/ContinuousDictionary.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP2/Quad2D.hpp"
#include "mesh/Elements.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP2;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Quad2D ETYPE;

struct LagrangeP1Quad2DFixture
{
  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Quad2DFixture()
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Quad2DFixture()
  {
  }
  /// common values accessed by all tests goes here

  const ETYPE::MappedCoordsT mapped_coords;
  const NodesT nodes;

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ETYPESuite, LagrangeP1Quad2DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  ETYPE::NodesT nodes;
  nodes <<
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0,
    0.5, 0.0,
    1.0, 0.5,
    0.5, 1.0,
    0.0, 0.5,
    0.5, 0.5;
  BOOST_CHECK_EQUAL(ETYPE::volume(nodes), 1.0);

  ETYPE::MappedCoordsT mapped_coord;
  ETYPE::CoordsT coord; coord << 0.5,0.5;
  ETYPE::compute_mapped_coordinate(coord,nodes,mapped_coord);
  CFinfo << mapped_coord.transpose() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

