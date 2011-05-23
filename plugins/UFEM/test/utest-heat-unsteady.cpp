// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for steady UFEM heat conduction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

 
#include "Common/Core.hpp"
#include "Common/XML/Protocol.hpp"
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

BOOST_AUTO_TEST_SUITE( HeatUnsteadySuite )

BOOST_AUTO_TEST_CASE( HeatLinearUnsteady )
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
  loader.load_library("coolfluid_mesh_neu");
  loader.load_library("coolfluid_mesh_gmsh");
  loader.load_library("coolfluid_field_generation");

  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  // One argument needed, containing the path to the meshes dir
  BOOST_CHECK_EQUAL(argc, 3);

  URI input_file = URI(argv[2]) / URI("ring2d-quads.neu");
  URI output_file("ring2d-unsteady.msh");

  CRoot& root = Core::instance().root();

  // Create the wizard
  Component::Ptr setup_ufem = build_component_abstract_type<Component>("CF.UFEM.SetupLinearSystem", "SetupUFEM");
  root.add_component(setup_ufem);

  // Build the model
  SignalFrame create_model_frame("", URI(), URI());
  SignalFrame& create_model_p = create_model_frame.map( Protocol::Tags::key_options() );

  create_model_p.set_option<std::string>("Model name", "UFEMHeat");
  create_model_p.set_option<std::string>("Solver", "CF.UFEM.HeatConductionLinearUnsteady");

  setup_ufem->call_signal("create_model", create_model_frame);

  Component::Ptr ufem_model = root.get_child_ptr("UFEMHeat");
  BOOST_CHECK(ufem_model);

  CMeshReader::Ptr mesh_reader = ufem_model->get_child_ptr("NeutralReader")->as_ptr<CMeshReader>();
  BOOST_CHECK(mesh_reader);

  CMesh::Ptr mesh = ufem_model->get_child_ptr("Domain")->create_component_ptr<CMesh>("Mesh");
  mesh_reader->read_from_to(input_file, *mesh);

  Component::Ptr ufem_method = ufem_model->get_child_ptr("LinearModel");
  BOOST_CHECK(ufem_method);

  Component::Ptr heat_eq = ufem_method->get_child_ptr("HeatEquation");
  BOOST_CHECK(heat_eq);

  ufem_model->get_child("LSS").configure_property("ConfigFile", std::string(argv[1]));
  
  // Configure region and variable names
  heat_eq->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads"));
  heat_eq->configure_property("TemperatureFieldName", std::string("Temperature"));
  heat_eq->configure_property("TemperatureVariableName", std::string("T"));

  // Set heat source field
  
  // Setup a constant field for the source term
  Component::Ptr heat_generator = build_component_abstract_type<Component>("CF.Tools.FieldGeneration.FieldGenerator", "HeatFieldGenerator");
  root.add_component(heat_generator);
  heat_generator->configure_property("FieldName", std::string("Heat"));
  heat_generator->configure_property("VariableName", std::string("q"));
  heat_generator->configure_property("Value", 0.);
  heat_generator->configure_property("Mesh", mesh->full_path());
  SignalFrame update_heat_frame("", URI(), URI());
  heat_generator->call_signal("update", update_heat_frame);
  
  heat_eq->configure_property("HeatFieldName", std::string("Heat"));
  heat_eq->configure_property("HeatVariableName", std::string("q"));

  // Material properties (copper)
  heat_eq->configure_property("k", /*398.*/1.);
  heat_eq->configure_property("alpha", /*11.57e-5*/1.);
  
  // Time stepping parameters
  ufem_method->configure_property("Timestep", 0.01);
  ufem_method->configure_property("StopTime", 0.2);
  
  // Initial condition value
  SignalFrame init_frame("", URI(), URI());
  SignalFrame& init_p = init_frame.map( Protocol::Tags::key_options() );

  init_p.set_option<std::string>("Name", "InitialTemperature");
  init_p.set_option<std::string>("FieldName", "Temperature");
  init_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_initial_condition", init_frame);
  Component::Ptr init = ufem_method->get_child_ptr("InitialTemperature");
  BOOST_CHECK(init);
  init->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology"));
  init->configure_property("InitialTemperature", 500.);
  
  // Inside boundary condition
  SignalFrame inside_bc_frame("", URI(), URI());
  SignalFrame& inside_bc_p = inside_bc_frame.map( Protocol::Tags::key_options() );

  inside_bc_p.set_option<std::string>("BCName", "Inside");
  inside_bc_p.set_option<std::string>("FieldName", "Temperature");
  inside_bc_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_dirichlet_bc", inside_bc_frame);
  Component::Ptr inside_bc = ufem_method->get_child_ptr("Inside");
  BOOST_CHECK(inside_bc);
  inside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads/inner"));
  inside_bc->configure_property("Inside", 300.);
  
  // Outside boundary condition
  SignalFrame outside_bc_frame("", URI(), URI());
  SignalFrame& outside_bc_p = outside_bc_frame.map( Protocol::Tags::key_options() );

  outside_bc_p.set_option<std::string>("BCName", "Outside");
  outside_bc_p.set_option<std::string>("FieldName", "Temperature");
  outside_bc_p.set_option<std::string>("VariableName", "T");

  ufem_method->call_signal("add_dirichlet_bc", outside_bc_frame);
  Component::Ptr outside_bc = ufem_method->get_child_ptr("Outside");
  BOOST_CHECK(outside_bc);
  outside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/topology/ring2d-quads/outer"));
  outside_bc->configure_property("Outside", 300.);
  
  // Run the solver
  SignalFrame run_frame("", URI(), URI());

  ufem_method->call_signal("initialize", run_frame);
  ufem_method->call_signal("run", run_frame);

  // Write the solution
  CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  ufem_model->add_component(writer);
  writer->configure_property( "fields", std::vector<URI>(1, URI("cpath://Root/UFEMHeat/Domain/Mesh/Temperature") ) );
  writer->write_from_to(*mesh, output_file);
}

BOOST_AUTO_TEST_SUITE_END()
