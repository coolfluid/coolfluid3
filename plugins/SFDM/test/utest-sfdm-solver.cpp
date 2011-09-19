// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Solver/CModel.hpp"
#include "Solver/Tags.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"

#include "SFDM/SFDSolver.hpp"

#include "Mesh/CRegion.hpp"
//#include "Mesh/CMesh.hpp"
//#include "Mesh/CField.hpp"
//#include "Mesh/CEntities.hpp"
//#include "Mesh/ElementType.hpp"
//#include "Mesh/CMeshWriter.hpp"
//#include "Mesh/CDomain.hpp"
//#include "Mesh/Actions/CInitFieldFunction.hpp"
//#include "Mesh/Actions/CreateSpaceP0.hpp"
//#include "Solver/CModelUnsteady.hpp"
//#include "Solver/CSolver.hpp"
//#include "Solver/CPhysicalModel.hpp"
//#include "Mesh/Actions/CBuildFaces.hpp"
//#include "Mesh/Actions/CBuildVolume.hpp"
//#include "Mesh/Actions/CreateSpaceP0.hpp"
//#include "SFDM/CreateSpace.hpp"

using namespace CF;
using namespace CF::Math;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;
using namespace CF::SFDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver_test )
{
  Core::instance().environment().configure_option("log_level", (Uint)INFO);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - NavierStokes 2D model

  CModel& model   = Core::instance().root().create_component<CModel>("model");
  model.setup("CF.SFDM.SFDSolver","CF.Physics.NavierStokes.NavierStokes2D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = model.solver().as_type<SFDSolver>();
  CDomain&   domain  = model.domain();

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_rectangle(mesh, 10., 10., 10, 10);
  solver.configure_option(SFDM::Tags::mesh(),mesh.uri());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.configure_option(SFDM::Tags::solution_vars(),std::string("CF.Physics.NavierStokes.Cons2D"));
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation

  // Initial condition
  Solver::Action& init_condition = solver.initial_conditions().create_initial_condition("shocktube");
  init_condition.configure_option(SFDM::Tags::input_vars(), physics.create_variables("CF.Physics.NavierStokes.Prim2D",SFDM::Tags::input_vars())->uri() );
  std::vector<std::string> functions;
  functions.push_back("if( x<=5 , 4.696  , 1.408  )"); // rho
  functions.push_back("if( x<=5 , 0      , 0      )"); // u
  functions.push_back("if( x<=5 , 0      , 0      )"); // v
  functions.push_back("if( x<=5 , 404400 , 101100 )"); // p
  init_condition.configure_option("functions",functions);
  solver.initial_conditions().execute();

  // Discretization
  solver.domain_discretization().create_cell_term("CF.SFDM.DummyCellTerm","term_1",std::vector<URI>(1,mesh.topology().uri()));
  solver.domain_discretization().create_cell_term("CF.SFDM.DummyCellTerm","term_2",std::vector<URI>(1,mesh.topology().uri()));
  solver.domain_discretization().create_cell_term("CF.SFDM.DummyCellTerm","term_3",std::vector<URI>(1,mesh.topology().uri()));

  // Time stepping
  solver.time_stepping().time().configure_option("time_step",0.1);
  solver.time_stepping().time().configure_option("end_time" ,0.1);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  CFinfo << model.tree() << CFendl;

  model.simulate();

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  //////////////////////////////////////////////////////////////////////////////
  // Output
  domain.write_mesh("sfdm_output.msh");


//  solver.configure_option_recursively("time",model.time().uri());
//  solver.configure_option_recursively("time_accurate",true);
//  solver.configure_option_recursively("cfl",1.);

//  model.time().configure_option("end_time",2.5);
//  model.time().configure_option("time_step",5.);

//  /// Initialize solution field with the function sin(2*pi*x)
//  Actions::CInitFieldFunction::Ptr init_field = Common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
//  //init_field->configure_option("functions",std::vector<std::string>(1,"sin(2*pi*x/10)"));

//  std::string gaussian="sigma:=1; mu:=5.; exp(-(x-mu)^2/(2*sigma^2)) / exp(-(mu-mu)^2/(2*sigma^2))";
//  init_field->configure_option("functions",std::vector<std::string>(1,gaussian));
//  init_field->configure_option("field",find_component_with_tag<CField>(mesh,"solution").uri());
//  init_field->transform(mesh);


//  std::vector<CField::Ptr> fields;
//  fields.push_back(find_component_with_tag<CField>(mesh,"solution").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"jacobian_determinant").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"residual").as_ptr<CField>());
//  fields.push_back(find_component_with_tag<CField>(mesh,"wave_speed").as_ptr<CField>());

//  CMeshWriter& gmsh_writer = solver.get_child("iterate").create_component("7_gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
//  gmsh_writer.configure_option("mesh",mesh.uri());
//  gmsh_writer.configure_option("file",URI("line_${iter}.msh"));
//  gmsh_writer.set_fields(fields);

//  gmsh_writer.execute();

//  CFinfo << model.tree() << CFendl;

//  //solver.get_child("iterate").configure_option("MaxIterations",1u);
//  solver.solve();

//  gmsh_writer.configure_option("file",URI("final.msh"));
//  gmsh_writer.execute();

//  /// write gmsh file. note that gmsh gets really confused because of the multistate view
////  gmsh_writer->write_from_to(mesh,"line_"+to_str(model.time().time())+".msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
