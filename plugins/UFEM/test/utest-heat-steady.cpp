// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for steady UFEM heat conduction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/LibLoader.hpp"
#include "Common/Log.hpp"
#include "Common/OSystem.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

BOOST_AUTO_TEST_SUITE( HeatSteadySuite )

BOOST_AUTO_TEST_CASE( HeatLinearSteady )
{
  // Load the required libraries (we assume the working dir is the binary path)
  LibLoader& loader = *OSystem::instance().lib_loader();

  const std::vector< boost::filesystem::path > lib_paths = boost::assign::list_of
                                                           ("../../../dso")
                                                           ("../src")
                                                           ("../../../src/Mesh/Neu")
                                                           ("../../../src/Mesh/Gmsh")
                                                           ("../../../src/Tools/FieldGeneration");
  loader.set_search_paths(lib_paths);

  loader.load_library("coolfluid_ufem");
  loader.load_library("coolfluid_mesh_neutral");
  loader.load_library("coolfluid_mesh_gmsh");
  loader.load_library("coolfluid_field_generation");

  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  // One argument needed, containing the path to the meshes dir
  BOOST_CHECK_EQUAL(argc, 3);

  boost::filesystem::path input_file = boost::filesystem::path(argv[2]) / boost::filesystem::path("ring2d-quads.neu");
  boost::filesystem::path output_file("ring2d-steady.msh");

  CRoot::Ptr root = Core::instance().root();

  Component::Ptr setup_ufem = create_component_abstract_type<Component>("CF.UFEM.SetupLinearSystem", "SetupUFEM");
  root->add_component(setup_ufem);

  SignalFrame create_model_frame("", URI(), URI());
  SignalFrame& create_model_p = create_model_frame.map( Protocol::Tags::key_options() );

  create_model_p.set_option<std::string>("Model name", "UFEMHeat");
  create_model_p.set_option<std::string>("Solver", "CF.UFEM.HeatConductionLinearSteady");

  setup_ufem->call_signal("create_model", create_model_frame);

  Component::Ptr ufem_model = root->get_child_ptr("UFEMHeat");
  BOOST_CHECK(ufem_model);

  CMeshReader::Ptr mesh_reader = ufem_model->get_child_ptr("NeutralReader")->as_ptr<CMeshReader>();
  BOOST_CHECK(mesh_reader);

  ufem_model->get_child("LSS").configure_property("ConfigFile", argv[1]);
  
  // Read the mesh
  CMesh::Ptr mesh = ufem_model->get_child_ptr("Domain")->create_component<CMesh>("Mesh");
  mesh_reader->read_from_to(input_file, mesh);
  
  // Setup a constant field for the source term
  Component::Ptr heat_generator = create_component_abstract_type<Component>("CF.Tools.FieldGeneration.FieldGenerator", "HeatFieldGenerator");
  root->add_component(heat_generator);
  heat_generator->configure_property("FieldName", std::string("Heat"));
  heat_generator->configure_property("VariableName", std::string("q"));
  heat_generator->configure_property("Value", 0.);
  heat_generator->configure_property("Mesh", mesh->full_path());
  SignalFrame update_heat_frame("", URI(), URI());
  heat_generator->call_signal("update", update_heat_frame);

  Component::Ptr ufem_method = ufem_model->get_child_ptr("LinearModel");
  BOOST_CHECK(ufem_method);

  Component::Ptr heat_eq = ufem_method->get_child_ptr("HeatEquation");
  BOOST_CHECK(heat_eq);

  heat_eq->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads"));
  BOOST_CHECK(true);
  heat_eq->configure_property("TemperatureFieldName", std::string("Temperature"));
  BOOST_CHECK(true);
  heat_eq->configure_property("TemperatureVariableName", std::string("T"));

  heat_eq->configure_property("HeatFieldName", std::string("Heat"));
  heat_eq->configure_property("HeatVariableName", std::string("q"));

  heat_eq->configure_property("k", 0.1);

  // Set intial field to 0
  SignalFrame init_frame("", URI(), URI());
  SignalFrame& init_p = init_frame.map( Protocol::Tags::key_options() );

  init_p.set_option<std::string>("Name", "InitialTemperature");
  init_p.set_option<std::string>("FieldName", "Temperature");
  init_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_initial_condition", init_frame);
  
  Component::Ptr init = ufem_method->get_child_ptr("InitialTemperature");
  BOOST_CHECK(init);
  init->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology"));
  init->configure_property("InitialTemperature", 0.);

  SignalFrame inside_bc_frame("", URI(), URI());
  SignalFrame& inside_bc_p = inside_bc_frame.map( Protocol::Tags::key_options() );

  inside_bc_p.set_option<std::string>("BCName", "Inside");
  inside_bc_p.set_option<std::string>("FieldName", "Temperature");
  inside_bc_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_dirichlet_bc", inside_bc_frame);
  Component::Ptr inside_bc = ufem_method->get_child_ptr("Inside");
  BOOST_CHECK(inside_bc);
  inside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads/inner"));
  inside_bc->configure_property("Inside", 273.);

  SignalFrame outside_bc_frame("", URI(), URI());
  SignalFrame& outside_bc_p = outside_bc_frame.map( Protocol::Tags::key_options() );

  outside_bc_p.set_option<std::string>("BCName", "Outside");
  outside_bc_p.set_option<std::string>("FieldName", "Temperature");
  outside_bc_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_dirichlet_bc", outside_bc_frame);
  Component::Ptr outside_bc = ufem_method->get_child_ptr("Outside");
  BOOST_CHECK(outside_bc);


  outside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads/outer"));

  outside_bc->configure_property("Outside", 500.);

  SignalFrame run_frame("", URI(), URI());

  ufem_method->call_signal("initialize", run_frame);

  ufem_method->call_signal("run", run_frame);

  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  ufem_model->add_component(writer);

  const std::vector<URI> field_uris = boost::assign::list_of
    ( URI("cpath://Root/UFEMHeat/Domain/Mesh/Temperature") );
    //( URI("cpath://Root/UFEMHeat/Domain/Mesh/Heat") );
  writer->configure_property( "Fields", field_uris );

  writer->write_from_to(mesh, output_file);

}

BOOST_AUTO_TEST_SUITE_END()
