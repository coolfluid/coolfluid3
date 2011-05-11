// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
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
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "SFDM/CreateSpace.hpp"

#include <boost/tuple/tuple.hpp>

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

boost::tuple<Component&,Uint> crazytest()
{
  return boost::tuple<Component&,Uint>(Core::instance().root(),5u);
}

BOOST_AUTO_TEST_CASE( Solver )
{


  CModelUnsteady& model   = Core::instance().root().create_component<CModelUnsteady>("model");
  CPhysicalModel& physics = model.create_physics("Physics");
  CDomain&        domain  = model.create_domain("Domain");
  CSolver&        solver  = model.create_solver("CF.SFDM.SFDSolver");


  Component& crazyroot = model;
  Uint crazynumber=0;

  boost::tuple<Component&,Uint> unified_cell = crazytest();
  crazytest().get<0>();
  crazytest().get<1>();
  /// Create a mesh consisting of a line with length 1. and 20 divisions
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(mesh, 1., 4);


  CGroup& tools = model.create_component<CGroup>("tools");
  tools.mark_basic();
  CMeshTransformer& spectral_difference_transformer = tools.create_component<CMeshTransformer>("SpectralFiniteDifferenceTransformer").mark_basic().as_type<CMeshTransformer>();
  spectral_difference_transformer.create_component<CBuildFaces>       ("1_build_faces").mark_basic().configure_property("store_cell2face",true);
  spectral_difference_transformer.create_component<CreateSpaceP0>     ("2_create_space_P0").mark_basic();
  spectral_difference_transformer.create_component<SFDM::CreateSpace> ("3_create_sfd_spaces").mark_basic().configure_property("P",2u);
  // spectral_difference_transformer.create_component<CBuildVolume>    ("4_build_volume_field")->mark_basic();

  spectral_difference_transformer.transform(mesh);

  solver.configure_property("physical_model",physics.full_path());
  solver.configure_property("Domain",domain.full_path());
  solver.configure_option_recursively("time",model.time().full_path());
  solver.configure_option_recursively("time_accurate",true);

  model.time().configure_property("end_time",0.001);
  model.time().configure_property("time_step",0.001);


  /// Initialize solution field with the function sin(2*pi*x)
  Actions::CInitFieldFunction::Ptr init_field = Common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
  init_field->configure_property("Functions",std::vector<std::string>(1,"sin(2*pi*x)"));
  init_field->configure_property("Field",find_component_with_tag<CField>(mesh,"solution").full_path());
  init_field->transform(mesh);


  solver.get_child("iterate").configure_property("verbose",true);
  solver.solve();

  CFinfo << model.tree() << CFendl;

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  boost::filesystem::path filename ("line.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,find_component_with_tag<CField>(mesh,"solution").as_ptr<CField>()));
  gmsh_writer->write_from_to(mesh.as_ptr<CMesh>(),filename);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

