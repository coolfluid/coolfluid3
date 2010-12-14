// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for steady UFEM heat conduction"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/XmlHelpers.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

BOOST_AUTO_TEST_SUITE( HeatSteadySuite )

BOOST_AUTO_TEST_CASE( HeatLinearSteady )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;
  
  // One argument needed, containing the path to the meshes dir
  BOOST_CHECK_EQUAL(argc, 2);
  
  boost::filesystem::path input_file = boost::filesystem::path(argv[1]) / boost::filesystem::path("ring2d-quads.neu");
  boost::filesystem::path output_file("ring2d-temp.msh");
  
  CRoot::Ptr root = Core::instance().root();
  
  Component::Ptr setup_ufem = create_component_abstract_type<Component>("CF.UFEM.SetupHeatConductionLinearSteady", "SetupUFEM");
  root->add_component(setup_ufem);
  
  boost::shared_ptr<XmlDoc> create_model_root = XmlOps::create_doc();
  XmlNode& create_model_node = *XmlOps::goto_doc_node(*create_model_root.get());
  XmlNode& create_model_frame = *XmlOps::add_signal_frame(create_model_node, "", "", "", true );
  XmlParams create_model_p ( create_model_frame );
  create_model_p.add_option<std::string>("Model name", "UFEMHeat");
  
  setup_ufem->call_signal("create_model", create_model_frame);
  
  Component::Ptr ufem_model = root->get_child("UFEMHeat");
  BOOST_CHECK(ufem_model);
  
  CMeshReader::Ptr mesh_reader = ufem_model->get_child<CMeshReader>("NeutralReader");
  BOOST_CHECK(mesh_reader);
  
  CMesh::Ptr mesh = ufem_model->get_child("Domain")->create_component<CMesh>("Mesh");
  mesh_reader->read_from_to(input_file, mesh);
  
  Component::Ptr ufem_method = ufem_model->get_child("HeatConductionLinearSteady");
  BOOST_CHECK(ufem_method);
  
  Component::Ptr heat_eq = ufem_method->get_child("HeatEquation");
  BOOST_CHECK(heat_eq);
  
  heat_eq->configure_property("ConductivityRegion", URI("cpath://Root/UFEMHeat/Domain/Mesh/ring2d-quads"));
  heat_eq->configure_property("TemperatureFieldName", std::string("Temperature"));
  heat_eq->configure_property("TemperatureVariableName", std::string("T"));
  
  boost::shared_ptr<XmlDoc> inside_bc_root = XmlOps::create_doc();
  XmlNode& inside_bc_node = *XmlOps::goto_doc_node(*inside_bc_root.get());
  XmlNode& inside_bc_frame = *XmlOps::add_signal_frame(inside_bc_node, "", "", "", true );
  XmlParams inside_bc_p ( inside_bc_frame );
  inside_bc_p.add_option<std::string>("BCName", "Inside");
  
  ufem_method->call_signal("add_dirichlet_bc", inside_bc_frame);
  Component::Ptr inside_bc = ufem_method->get_child("Inside");
  BOOST_CHECK(inside_bc);
  inside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/ring2d-quads/inner"));
  inside_bc->configure_property("Inside", 10.);
  
  boost::shared_ptr<XmlDoc> outside_bc_root = XmlOps::create_doc();
  XmlNode& outside_bc_node = *XmlOps::goto_doc_node(*outside_bc_root.get());
  XmlNode& outside_bc_frame = *XmlOps::add_signal_frame(outside_bc_node, "", "", "", true );
  XmlParams outside_bc_p ( outside_bc_frame );
  outside_bc_p.add_option<std::string>("BCName", "Outside");
  
  ufem_method->call_signal("add_dirichlet_bc", outside_bc_frame);
  Component::Ptr outside_bc = ufem_method->get_child("Outside");
  BOOST_CHECK(outside_bc);
  outside_bc->configure_property("Region", URI("cpath://Root/UFEMHeat/Domain/Mesh/ring2d-quads/outer"));
  outside_bc->configure_property("Outside", 35.);
  
  boost::shared_ptr<XmlDoc> run_root = XmlOps::create_doc();
  XmlNode& run_node = *XmlOps::goto_doc_node(*run_root.get());
  XmlNode& run_frame = *XmlOps::add_signal_frame(run_node, "", "", "", true );
  
  ufem_method->call_signal("run", run_frame);
  
  CMeshWriter::Ptr writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  ufem_model->add_component(writer);
  writer->write_from_to(mesh, output_file);
}

BOOST_AUTO_TEST_SUITE_END()