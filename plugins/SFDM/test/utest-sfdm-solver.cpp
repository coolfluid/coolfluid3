// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
 
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/FindComponents.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "Solver/CModelUnsteady.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/Actions/CBuildVolume.hpp"
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "SFDM/CreateSpace.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
//using namespace CF::Solver::Actions;
using namespace CF::SFDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver )
{
  Core::instance().environment().configure_property("log_level", (Uint)INFO);

  CModelUnsteady& model   = Core::instance().root().create_component<CModelUnsteady>("model");
  CPhysicalModel& physics = model.create_physics("Physics");
  CDomain&        domain  = model.create_domain("Domain");
  CSolver&        solver  = model.create_solver("CF.SFDM.SFDSolver");

  //////////////////////////////////////////////////////////////////////////////
  // configure physics

  physics.configure_property("solution_state",std::string("CF.AdvectionDiffusion.State"));


  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  /// Create a mesh consisting of a line with length 1. and 20 divisions
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  const Uint divisions=100;
  const Real length=10.;
  CSimpleMeshGenerator::create_line(mesh, length, divisions);


  CGroup& tools = model.create_component<CGroup>("tools");
  tools.mark_basic();
  CMeshTransformer& spectral_difference_transformer = tools.create_component<CMeshTransformer>("SpectralFiniteDifferenceTransformer").mark_basic().as_type<CMeshTransformer>();
  spectral_difference_transformer.create_component<CBuildFaces>       ("1_build_faces").mark_basic().configure_property("store_cell2face",true);
  spectral_difference_transformer.create_component<CreateSpaceP0>     ("2_create_space_P0").mark_basic();
  spectral_difference_transformer.create_component<SFDM::CreateSpace> ("3_create_sfd_spaces").mark_basic().configure_property("P",0u);
  spectral_difference_transformer.create_component<CBuildVolume>      ("4_build_volume_field").mark_basic();

  spectral_difference_transformer.transform(mesh);

  //////////////////////////////////////////////////////////////////////////////
  // configure solver

  solver.configure_property("physical_model",physics.uri());
  solver.configure_property("domain",domain.uri());
  solver.configure_option_recursively("riemann_solver",std::string("CF.RiemannSolvers.Roe"));
  solver.configure_option_recursively("roe_state",std::string("CF.AdvectionDiffusion.State"));
  solver.configure_option_recursively("solution_state",physics.solution_state().uri());


  solver.configure_option_recursively("time",model.time().uri());
  solver.configure_option_recursively("time_accurate",true);
  solver.configure_option_recursively("cfl",1.);

  model.time().configure_property("end_time",2.5);
  model.time().configure_property("time_step",5.);

  /// Initialize solution field with the function sin(2*pi*x)
  Actions::CInitFieldFunction::Ptr init_field = Common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
  //init_field->configure_property("Functions",std::vector<std::string>(1,"sin(2*pi*x/10)"));

  std::string gaussian="sigma:=1; mu:=5.; exp(-(x-mu)^2/(2*sigma^2)) / exp(-(mu-mu)^2/(2*sigma^2))";
  init_field->configure_property("functions",std::vector<std::string>(1,gaussian));
  init_field->configure_property("field",find_component_with_tag<CField>(mesh,"solution").uri());
  init_field->transform(mesh);


  std::vector<CField::Ptr> fields;
  fields.push_back(find_component_with_tag<CField>(mesh,"solution").as_ptr<CField>());
  fields.push_back(find_component_with_tag<CField>(mesh,"jacobian_determinant").as_ptr<CField>());
  fields.push_back(find_component_with_tag<CField>(mesh,"residual").as_ptr<CField>());
  fields.push_back(find_component_with_tag<CField>(mesh,"wave_speed").as_ptr<CField>());

  CMeshWriter& gmsh_writer = solver.get_child("iterate").create_component("7_gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
  gmsh_writer.configure_property("mesh",mesh.uri());
  gmsh_writer.configure_property("file",URI("line_${iter}.msh"));
  gmsh_writer.set_fields(fields);

  gmsh_writer.execute();

  CFinfo << model.tree() << CFendl;

  //solver.get_child("iterate").configure_property("MaxIterations",1u);
  solver.solve();

  gmsh_writer.configure_property("file",URI("final.msh"));
  gmsh_writer.execute();

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
//  gmsh_writer->write_from_to(mesh,"line_"+to_str(model.time().time())+".msh");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
