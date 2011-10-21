// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::Actions::CBuildFaces"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"

#include "common/FindComponents.hpp"

#include "mesh/Actions/CreateSpaceP0.hpp"
#include "mesh/Actions/CBuildFaces.hpp"
#include "mesh/Actions/CBuildFaceNormals.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/CFaces.hpp"
#include "mesh/CCellFaces.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/Field.hpp"
#include "mesh/CFaceCellConnectivity.hpp"
#include "mesh/CCells.hpp"
#include "mesh/CSimpleMeshGenerator.hpp"

using namespace cf3;
using namespace boost::assign;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::Actions;

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
  static Mesh::Ptr mesh;
};

Mesh::Ptr TestCBuildFaces_Fixture::mesh = Core::instance().root().create_component_ptr<Mesh>("mesh");

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
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->read_mesh_into("quadtriag.neu",*mesh);

  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");

  facebuilder->set_mesh(mesh);
  facebuilder->execute();

  //CFinfo << mesh->tree() << CFendl;

  MeshTransformer::Ptr info = build_component_abstract_type<MeshTransformer>("CF.Mesh.Actions.CInfo","info");
  //info->transform(mesh);

  CRegion& wall_region = find_component_recursively_with_name<CRegion>(mesh->topology(),"wall");
  CFaces& wall_faces = find_component<CFaces>(wall_region);
  CFaceCellConnectivity& f2c = find_component<CFaceCellConnectivity>(wall_faces);
  Component::Ptr cells;
  Uint cell_idx(0);

  CFinfo << "\n\nCHECKING wall connectivity"<<CFendl;
  for (Uint face=0; face<f2c.size(); ++face)
  {
    CFinfo << wall_faces.parent().name()<<"/"<<wall_faces.name() << "["<<face<<"] <--> ";

    boost::tie(cells,cell_idx) = f2c.lookup().location(f2c.connectivity()[face][0]);
    CFinfo << cells->parent().parent().name()<<"/"<<cells->name() << "["<<cell_idx<<"]" << CFendl;
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
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_face_normals )
{
  allocate_component<CreateSpaceP0>("create_space_P0")->transform(mesh);
  BOOST_CHECK(true);
  CBuildFaceNormals::Ptr face_normal_builder = allocate_component<CBuildFaceNormals>("facenormalsbuilder");

  face_normal_builder->set_mesh(mesh);
  face_normal_builder->execute();

  //CFinfo << mesh->tree() << CFendl;

  MeshTransformer::Ptr info = build_component_abstract_type<MeshTransformer>("CF.Mesh.Actions.CInfo","info");
  //info->transform(mesh);

  MeshWriter::Ptr mesh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","writer");

  mesh_writer->set_fields(std::vector<Field::Ptr>(1,find_component_ptr_recursively_with_name<Field>(*mesh,mesh::Tags::normal())));
  mesh_writer->write_from_to(*mesh,"facenormals.msh");
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces_rectangle )
{
  BOOST_CHECK(true);

  CSimpleMeshGenerator::Ptr mesh_gen = allocate_component<CSimpleMeshGenerator>("mesh_gen");
  std::vector<Real> lengths  = list_of(10.)(10.);
  std::vector<Uint> nb_cells = list_of(5u)(5u);
  mesh_gen->configure_option("mesh",URI("//Root/rectangle_mesh"))
      .configure_option("lengths",lengths)
      .configure_option("nb_cells",nb_cells);
  Mesh& rmesh = mesh_gen->generate();
  BOOST_CHECK(true);
  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  BOOST_CHECK(true);

  facebuilder->set_mesh(rmesh);
  BOOST_CHECK(true);

  facebuilder->execute();

  BOOST_CHECK(true);
  CRegion& inner_faces_region = find_component_recursively_with_name<CRegion>(rmesh.topology(),mesh::Tags::inner_faces());
  CCellFaces& inner_faces = find_component<CCellFaces>(inner_faces_region);
  CFaceCellConnectivity& f2c = find_component<CFaceCellConnectivity>(inner_faces);
  Component::Ptr cells;
  Uint cell_idx(0);
  BOOST_CHECK(true);

  CFinfo << "\n\nCHECKING inner faces connectivity"<<CFendl;
  for (Uint face=0; face<f2c.size(); ++face)
  {
    CFinfo << inner_faces.parent().name()<<"/"<<inner_faces.name() << "["<<face<<"] <--> ";

    boost::tie(cells,cell_idx) = f2c.lookup().location(f2c.connectivity()[face][0]);
    CFinfo << cells->parent().parent().name()<<"/"<<cells->name() << "["<<cell_idx<<"]  <-->  ";
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
    boost::tie(cells,cell_idx) = f2c.lookup().location(f2c.connectivity()[face][1]);
    CFinfo << cells->parent().parent().name()<<"/"<<cells->name() << "["<<cell_idx<<"]" << CFendl;
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

