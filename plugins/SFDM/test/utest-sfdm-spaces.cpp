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
#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "SFDM/Core/CreateSpace.hpp"


using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::SFDM;
using namespace CF::SFDM::Core;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( LineP1 )
{
  CMesh::Ptr mesh = Common::Core::instance().root().create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(*mesh, 1., 20);


  /// This is the standard LagrangeP1 space[0], coming with the element type
  CField::Ptr mesh_solution = mesh->create_component<CField>("mesh_solution");
  mesh_solution->configure_property("Space",0u);
  mesh_solution->configure_property("Topology",mesh->topology().full_path());
  mesh_solution->configure_property("FieldType",std::string("CellBased"));
  mesh_solution->configure_property("VarNames",std::vector<std::string>(1,"solution"));
  mesh_solution->configure_property("VarTypes",std::vector<std::string>(1,"scalar"));
  mesh_solution->create_data_storage();


  /// Create a space[1] for solution of order P2, and thus a space[2] for flux P3
  CreateSpace::Ptr space_creator = allocate_component<CreateSpace>("space_creator");
  space_creator->configure_property("P",2u);
  space_creator->transform(*mesh);


  /// This is a field with space[1]
  CField::Ptr solution = mesh->create_component<CField>("solution");
  solution->configure_property("Space",1u);
  solution->configure_property("Topology",mesh->topology().full_path());
  solution->configure_property("FieldType",std::string("CellBased"));
  solution->configure_property("VarNames",std::vector<std::string>(1,"solution"));
  solution->configure_property("VarTypes",std::vector<std::string>(1,"scalar"));
  solution->create_data_storage();

  //CFinfo << mesh->tree() << CFendl;
  CFinfo << "elements = " << mesh->topology().recursive_elements_count() << CFendl;
  CFinfo << "mesh_solution_fieldsize = " << mesh_solution->size() << CFendl;
  CFinfo << "solution_fieldsize = " << solution->size() << CFendl;

  /// Initialize solution field
  Actions::CInitFieldFunction::Ptr init_field = Common::Core::instance().root().create_component<Actions::CInitFieldFunction>("init_field");
  init_field->configure_property("Functions",std::vector<std::string>(1,"sin(2*pi*x)"));
  init_field->configure_property("Field",solution->full_path());
  init_field->transform(*mesh);

  CFinfo << "solution data:\n" << solution->data() << CFendl;

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  boost::filesystem::path fp_out ("line.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,solution));
  gmsh_writer->write_from_to(mesh,fp_out);


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

