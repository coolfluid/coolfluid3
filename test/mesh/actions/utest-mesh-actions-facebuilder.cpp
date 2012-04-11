// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::BuildFaces"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/FindComponents.hpp"

#include "mesh/actions/BuildFaces.hpp"
#include "mesh/actions/BuildFaceNormals.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/Region.hpp"
#include "mesh/Faces.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/Field.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/CellFaces.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace boost::assign;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;

////////////////////////////////////////////////////////////////////////////////

struct TestBuildFaces_Fixture
{
  /// common setup for each test case
  TestBuildFaces_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestBuildFaces_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  int m_argc;
  char** m_argv;


  /// common values accessed by all tests goes here
  static Handle< Mesh > mesh;
};

Handle< Mesh > TestBuildFaces_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestBuildFaces_TestSuite, TestBuildFaces_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  boost::shared_ptr<BuildFaces> facebuilder = allocate_component<BuildFaces>("facebuilder");
  BOOST_CHECK_EQUAL(facebuilder->name(),"facebuilder");
  Core::instance().environment().options().configure_option("log_level",(Uint)INFO);
}

BOOST_AUTO_TEST_CASE( build_faceconnectivity )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  meshreader->read_mesh_into("../../../resources/quadtriag.neu",*mesh);

  FaceCellConnectivity& f2c = *mesh->create_component<FaceCellConnectivity>("faces");
  f2c.setup(mesh->topology());

  BOOST_CHECK_EQUAL( f2c.size() , 31u);

  Face2Cell face(f2c);
  Handle<Elements> liquid_triag (mesh->access_component("topology/quadtriag/liquid/Triag"));
  Handle<Elements> gas_triag    (mesh->access_component("topology/quadtriag/gas/Triag"));
  Handle<Elements> gas_quad     (mesh->access_component("topology/quadtriag/gas/Quad"));

  face.idx=0;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,9) );

  face.idx=1;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,2) );

  face.idx=2;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,1) );

  face.idx=3;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,1) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,2) );

  face.idx=4;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,1) );

  face.idx=5;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,2) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,3) );

  face.idx=6;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,2) );

  face.idx=7;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,3) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,8) );

  face.idx=8;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,3) );

  face.idx=9;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,4) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,5) );

  face.idx=10;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,4) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,0) );

  face.idx=11;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,4) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,9) );

  face.idx=12;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,5) );

  face.idx=13;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,5) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,6) );

  face.idx=14;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,6) );

  face.idx=15;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,6) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,7) );

  face.idx=16;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,7) );

  face.idx=17;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,7) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,8) );

  face.idx=18;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*liquid_triag,8) );
  BOOST_CHECK( face.cells()[1] == Entity(*liquid_triag,9) );

  face.idx=19;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,0) );

  face.idx=20;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,0) );

  face.idx=21;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_quad,1) );

  face.idx=22;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,1) );

  face.idx=23;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,1) );

  face.idx=24;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,1) );

  face.idx=25;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_quad,1) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,3) );

  face.idx=26;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_triag,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,3) );

  face.idx=27;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_triag,0) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,1) );

  face.idx=28;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_triag,1) );

  face.idx=29;
  BOOST_CHECK_EQUAL( face.is_bdry() , true);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_triag,2) );

  face.idx=30;
  BOOST_CHECK_EQUAL( face.is_bdry() , false);
  BOOST_CHECK( face.cells()[0] == Entity(*gas_triag,2) );
  BOOST_CHECK( face.cells()[1] == Entity(*gas_triag,3) );

//  for (Face2Cell face(f2c); face.idx<f2c.size(); ++face.idx)
//  {
//    CFinfo << face.idx << "  : " << face.cells()[0];
//    if (face.is_bdry())
//      CFinfo << CFendl;
//    else
//      CFinfo << "   <-->   " << face.cells()[1] << CFendl;
//  }

//  CFinfo << mesh->tree() << CFendl<<CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces )
{
  boost::shared_ptr<BuildFaces> facebuilder = allocate_component<BuildFaces>("facebuilder");

  facebuilder->set_mesh(mesh);
  facebuilder->execute();

  //CFinfo << mesh->tree() << CFendl;

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info","info");
  //info->transform(mesh);

  Region& wall_region = find_component_recursively_with_name<Region>(mesh->topology(),"wall");
  Faces& wall_faces = find_component<Faces>(wall_region);
  FaceCellConnectivity& f2c = find_component<FaceCellConnectivity>(wall_faces);
  Handle< Component > cells;
  Uint cell_idx(0);

  CFinfo << "\n\nCHECKING wall connectivity"<<CFendl;
  BOOST_CHECK_EQUAL(f2c.size(),6u);
  for (Face2Cell face(f2c); face.idx<f2c.size(); ++face.idx)
  {
    CFinfo << wall_faces.parent()->name()<<"/"<<wall_faces.name() << "["<<face.idx<<"] <--> ";

    Entity cell = face.cells()[0];
    CFinfo << cell << CFendl;
    RealMatrix cell_coordinates = cell.get_coordinates();
    RealVector face_coordinates = wall_faces.geometry_space().get_coordinates(face.idx).row(0);
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
  boost::shared_ptr<BuildFaceNormals> face_normal_builder = allocate_component<BuildFaceNormals>("facenormalsbuilder");

  face_normal_builder->set_mesh(mesh);
  face_normal_builder->execute();

  //CFinfo << mesh->tree() << CFendl;

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info","info");
  //info->transform(mesh);

  boost::shared_ptr< MeshWriter > mesh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","writer");

  std::vector<URI> fields(1,find_component_ptr_recursively_with_name<Field>(*mesh,mesh::Tags::normal())->uri());
  mesh_writer->options().configure_option("fields",fields);
  mesh_writer->options().configure_option("mesh",mesh);
  mesh_writer->options().configure_option("file",URI("facenormals.msh"));
  mesh_writer->execute();
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces_rectangle )
{
  BOOST_CHECK(true);

  boost::shared_ptr<SimpleMeshGenerator> mesh_gen = allocate_component<SimpleMeshGenerator>("mesh_gen");
  std::vector<Real> lengths  = list_of(10.)(10.);
  std::vector<Uint> nb_cells = list_of(5u)(5u);
  mesh_gen->options().configure_option("mesh",URI("//rectangle_mesh"));
  mesh_gen->options().configure_option("lengths",lengths);
  mesh_gen->options().configure_option("nb_cells",nb_cells);
  Mesh& rmesh = mesh_gen->generate();
  BOOST_CHECK(true);
  boost::shared_ptr<BuildFaces> facebuilder = allocate_component<BuildFaces>("facebuilder");
  BOOST_CHECK(true);

  facebuilder->set_mesh(rmesh);
  BOOST_CHECK(true);

  facebuilder->execute();

  BOOST_CHECK(true);
  Region& inner_faces_region = find_component_recursively_with_name<Region>(rmesh.topology(),mesh::Tags::inner_faces());
  CellFaces& inner_faces = find_component<CellFaces>(inner_faces_region);
  FaceCellConnectivity& f2c = find_component<FaceCellConnectivity>(inner_faces);
  Handle< Component > cells;
  Uint cell_idx(0);
  BOOST_CHECK(true);

  CFinfo << "\n\nCHECKING inner faces connectivity"<<CFendl;
  for (Face2Cell face(f2c); face.idx<f2c.size(); ++face.idx)
  {
    CFinfo << inner_faces.parent()->name()<<"/"<<inner_faces.name() << "["<<face.idx<<"] <--> ";

    Entity cell = face.cells()[0];
    CFinfo << cell.comp->parent()->parent()->name()<<"/"<<cell.comp->name() << "["<<cell.idx<<"]  <-->  ";
    RealMatrix cell_coordinates = cell.get_coordinates();
    RealVector face_coordinates = inner_faces.geometry_space().get_coordinates(face.idx).row(0);
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
    cell = face.cells()[0];
    CFinfo << cell.comp->parent()->parent()->name()<<"/"<<cell.comp->name() << "["<<cell.idx<<"]"<<CFendl;
    cell_coordinates = cell.get_coordinates();
    face_coordinates = inner_faces.geometry_space().get_coordinates(face.idx).row(0);
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

