// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::FVM::BuildGhostStates"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/Actions/CBuildFaceNormals.hpp"
#include "Mesh/Actions/CExtract.hpp"
#include "Mesh/CCells.hpp"


#include "FVM/BuildGhostStates.hpp"

using namespace boost;
using namespace boost::assign;
using namespace boost::filesystem;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::FVM;
using namespace CF::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FVM_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_buildghoststates )
{
  CMesh::Ptr mesh = Core::instance().root()->create_component<CMesh>("mesh");
  
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  boost::filesystem::path fp_in("quadtriag.neu");
  //meshreader->read_from_to(fp_in,mesh);
  
  //Tools::MeshGeneration::create_line(*mesh, 10. , 10);
  Tools::MeshGeneration::create_rectangle(*mesh, 10. , 10., 4 , 4 );
  
  CExtract::Ptr extract = allocate_component<CExtract>("extract");
  std::vector<std::string> regions;
  regions.push_back("wall");
  regions.push_back("inlet");
  //regions.push_back("outlet");
  regions.push_back("gas");
  extract->configure_property("Regions",regions);
//  extract->transform(mesh);
  
  CBuildFaces::Ptr build_faces = allocate_component<CBuildFaces>("build_faces");
  build_faces->transform(mesh);

  CBuildFaceNormals::Ptr build_facenormals = allocate_component<CBuildFaceNormals>("build_facenormals");
  build_facenormals->transform(mesh);
  
  BuildGhostStates::Ptr build_gstates = allocate_component<BuildGhostStates>("build_gstates");
  build_gstates->transform(mesh);
  
  CFinfo << mesh->tree() << CFendl;
  // boost_foreach(CRegion& boundary_faces, find_components_recursively_with_name<CRegion>(mesh->topology(),"boundary_faces"))
  // {
  //   boost_foreach(CFaces& faces, find_components<CFaces>(boundary_faces))
  //   {
  //     CFinfo << faces.full_path().path() << CFendl;;
  //     CFaceCellConnectivity& face2cell = find_component<CFaceCellConnectivity>(faces);
  //     for (Uint face=0; face<faces.size(); ++face)
  //     {
  //       CTable<Uint>::ConstRow elements = face2cell.elements(face);
  //       CFinfo << "  face " << face << " connected to : " << to_vector(elements).transpose() << CFendl;
  //       CCells::Ptr cells;
  //       Uint cell_idx(0);
  //       boost_foreach (Uint elem, elements)
  //       {  
  //         boost::tie(cells,cell_idx) = face2cell.element_location(elem);
  //         CFinfo << "     " << cells->full_path().path() << "["<<cell_idx<<"]" << CFendl ;
  //       }
  //     }
  //   }
  // }

  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  
  std::vector<CField::Ptr> fields;
  boost_foreach(CField& field, find_components_recursively<CField>(*mesh))
    fields.push_back(field.as_ptr<CField>());
  meshwriter->set_fields(fields);
  
  
	boost::filesystem::path fp_out("line_ghosts.msh");
	meshwriter->write_from_to(mesh,fp_out);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

