// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests Mesh::Actions::CBuildFaces"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/Actions/CBuildFaceNormals.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCellFaces.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;

////////////////////////////////////////////////////////////////////////////////

struct TestCBuildFaces_Fixture
{
  /// common setup for each test case
  TestCBuildFaces_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestCBuildFaces_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  int m_argc;
  char** m_argv;


  /// common values accessed by all tests goes here
  static CMesh::Ptr mesh;
};

CMesh::Ptr TestCBuildFaces_Fixture::mesh = allocate_component<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCBuildFaces_TestSuite, TestCBuildFaces_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  BOOST_CHECK_EQUAL(facebuilder->name(),"facebuilder");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("quadtriag.neu");
  meshreader->read_from_to(fp_in,mesh);
  
  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  
  facebuilder->set_mesh(mesh);
  facebuilder->execute();
  
  //CFinfo << mesh->tree() << CFendl;
  
  CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  //info->transform(mesh);
  
  CRegion& wall_region = find_component_recursively_with_name<CRegion>(mesh->topology(),"wall");
  CFaces& wall_faces = find_component<CFaces>(wall_region);
  CFaceCellConnectivity& f2c = find_component<CFaceCellConnectivity>(wall_faces);
  Component::Ptr cells;
  Uint cell_idx(0);
  
  CFinfo << "\n\nCHECKING"<<CFendl;
  for (Uint face=0; face<f2c.size(); ++face)
  {
    CFinfo << wall_faces.parent()->name()<<"/"<<wall_faces.name() << "["<<face<<"] <--> ";
    
    boost::tie(cells,cell_idx) = f2c.cells().location(f2c.connectivity()[face][0]);
    CFinfo << cells->parent()->parent()->name()<<"/"<<cells->name() << "["<<cell_idx<<"]" << CFendl;
    RealMatrix cell_coordinates = cells->as_type<CElements>().get_coordinates(cell_idx);
    RealVector face_coordinates = wall_faces.get_coordinates(face).row(0);
    bool match_found = false;
    for (Uint i=0; i<cell_coordinates.rows(); ++i)
    {
      if (cell_coordinates.row(i) == face_coordinates.transpose())
      {
        match_found = true;
        break;
      }
    }
    BOOST_CHECK(match_found);
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_face_normals )
{
  
  CBuildFaceNormals::Ptr face_normal_builder = allocate_component<CBuildFaceNormals>("facenormalsbuilder");
  
  face_normal_builder->set_mesh(mesh);
  face_normal_builder->execute();
  
  //CFinfo << mesh->tree() << CFendl;
  
  CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  //info->transform(mesh);
  
  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","writer");

  boost::filesystem::path file ("facenormals.msh");
  mesh_writer->set_fields(std::vector<CField::Ptr>(1,find_component_ptr<CField>(*mesh)));
  mesh_writer->write_from_to(mesh,file);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces_rectangle )
{
  CMesh::Ptr rmesh = Core::instance().root()->create_component<CMesh>("rectangle_mesh");
  CSimpleMeshGenerator::create_rectangle(*rmesh, 10. , 10., 50 , 50 );
  
  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  
  facebuilder->set_mesh(rmesh);
  facebuilder->execute();
  
  CRegion& inner_faces_region = find_component_recursively_with_name<CRegion>(rmesh->topology(),"inner_faces");
  CCellFaces& inner_faces = find_component<CCellFaces>(inner_faces_region);
  CFaceCellConnectivity& f2c = find_component<CFaceCellConnectivity>(inner_faces);
  Component::Ptr cells;
  Uint cell_idx(0);
  
  CFinfo << "\n\nCHECKING"<<CFendl;
  for (Uint face=0; face<f2c.size(); ++face)
  {
    CFinfo << inner_faces.parent()->name()<<"/"<<inner_faces.name() << "["<<face<<"] <--> ";
    
    boost::tie(cells,cell_idx) = f2c.cells().location(f2c.connectivity()[face][0]);
    CFinfo << cells->parent()->parent()->name()<<"/"<<cells->name() << "["<<cell_idx<<"]  <-->  ";
    RealMatrix cell_coordinates = cells->as_type<CElements>().get_coordinates(cell_idx);
    RealVector face_coordinates = inner_faces.get_coordinates(face).row(0);
    bool match_found = false;
    for (Uint i=0; i<cell_coordinates.rows(); ++i)
    {
      if (cell_coordinates.row(i) == face_coordinates.transpose())
      {
        match_found = true;
        break;
      }
    }
    BOOST_CHECK(match_found);
    
    match_found = false;
    boost::tie(cells,cell_idx) = f2c.cells().location(f2c.connectivity()[face][1]);
    CFinfo << cells->parent()->parent()->name()<<"/"<<cells->name() << "["<<cell_idx<<"]" << CFendl;
    cell_coordinates = cells->as_type<CElements>().get_coordinates(cell_idx);
    face_coordinates = inner_faces.get_coordinates(face).row(0);
    for (Uint i=0; i<cell_coordinates.rows(); ++i)
    {
      if (cell_coordinates.row(i) == face_coordinates.transpose())
      {
        match_found = true;
        break;
      }
    }
    BOOST_CHECK(match_found);
    
  }

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

