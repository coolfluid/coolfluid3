// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/foreach.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/proto/debug.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP2/Triag2D.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"

#include "physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;


BOOST_AUTO_TEST_SUITE( ProtoLagrangeP2Suite )

using boost::proto::lit;

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( LagrangeP2 )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("AccessGrid");
  Tools::MeshGeneration::create_rectangle_tris(*mesh, 1., 1., 1, 1);

  Dictionary& dict = mesh->create_continuous_space("cf3.mesh.LagrangeP2", "cf3.mesh.LagrangeP2");
  dict.add_tag("solution");
  dict.create_field( "solution", "u[vector]" ).add_tag("solution");

  FieldVariable<0, VectorField > u("u", "solution", "cf3.mesh.LagrangeP2");
  
  RealVector2 centroid(1./3, 1.3);
  RealMatrix result;
  
  RealMatrix expected(6,6);
  expected <<
    1./60., -1./360., -1./360., 0., -1./90., 0.,
    -1./360., 1./60., -1./360., 0., 0., -1./90.,
    -1./360., -1./360., 1./60., -1./90., 0., 0.,
    0., 0., -1./90., 4./45., 2./45., 2./45.,
    -1./90., 0., 0., 2./45., 4./45., 2./45.,
    0., -1./90., 0., 2./45., 2./45., 4./45.;

  for_each_node(mesh->topology(), group(u[0] = 0., u[1] = 1.));

  for_each_element< boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP2::Triag2D> >
  (
    mesh->topology(),
    group
    (
      _A(u) = _0,
      element_quadrature(_A(u[_i], u[_i]) += transpose(N(u))*N(u)),
      lit(result) = _A
    )
  );

  BOOST_CHECK(((result.block<6,6>(0,0) - expected).array().abs() < 1e-15).all());
  BOOST_CHECK(((result.block<6,6>(6,6) - expected).array().abs() < 1e-15).all());
  BOOST_CHECK((result.block<6,6>(0,6).array() == 0.).all());
  BOOST_CHECK((result.block<6,6>(6,0).array() == 0.).all());
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
