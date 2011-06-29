// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RiemannSolvers"

#include <boost/test/unit_test.hpp>


#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "Mesh/Types.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/Actions/CreateSpaceP0.hpp"

#include "Solver/CTime.hpp"

#include "RungeKutta/RK.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::RungeKutta;
using namespace CF::Solver;
using namespace CF::Mesh;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_RK )
{
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(mesh,1.,10);
  allocate_component<Mesh::Actions::CreateSpaceP0>("create_space[0]")->transform(mesh);
  CField& solution = mesh.create_field("solution",CField::Basis::CELL_BASED);
  CField& residual = mesh.create_field("residual",solution);
  CField& update_coeff = mesh.create_scalar_field("update_coeff",solution);

  CTime& time = Core::instance().root().create_component<CTime>("time");

  CAction& rk4 = Core::instance().root().create_component("RK4","CF.RungeKutta.RK").as_type<CAction>();
  rk4.configure_option("stages",4u);
  rk4.configure_option_recursively("mesh",mesh.uri());
  rk4.configure_option_recursively("time",time.uri());
  rk4.configure_option("solution",solution.uri());
  rk4.configure_option("residual",residual.uri());
  rk4.configure_option("update_coeff",update_coeff.uri());
  rk4.execute();

  CFinfo << rk4.tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

