// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CNodes.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct FieldTests_Fixture
{
  /// common setup for each test case
  FieldTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~FieldTests_Fixture()
  {
  }

  /// common mesh accessed by all tests
  static CMesh::Ptr m_mesh;
};

CMesh::Ptr FieldTests_Fixture::m_mesh;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldTests_TestSuite, FieldTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshCreation )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");
  // the mesh to store in
  m_mesh = meshreader->create_mesh_from(fp_in);
  
  Core::instance().root()->add_component(m_mesh);

}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CoordinatesFieldCreation )
{
	CMesh& mesh = *m_mesh;

  std::vector<std::string> names;
  std::vector<std::string> types;

  CField2& coordinates = *mesh.create_component<CField2>("coordinates");
  names = list_of("coordinates");
  types = list_of("Vector2D");
  coordinates.configure_property("Topology",mesh.topology().full_path());
  coordinates.configure_property("VarNames",names);
  coordinates.configure_property("VarTypes",types);
  coordinates.configure_property("FieldType",std::string("PointBased"));
  coordinates.configure_property("Space",0u);
  coordinates.create_data_storage();

  BOOST_CHECK_EQUAL( coordinates.basis() , CField2::Basis::POINT_BASED );
  BOOST_CHECK_EQUAL( coordinates.var_name() , std::string("coordinates") );
  BOOST_CHECK_EQUAL( coordinates.var_name(0) , std::string("coordinates") );
  BOOST_CHECK_EQUAL( coordinates.var_index("coordinates") , 0u );
  BOOST_CHECK_EQUAL( coordinates.var_number("coordinates") , 0u );
  BOOST_CHECK_EQUAL( coordinates.var_type() , CField2::VECTOR_2D );
  BOOST_CHECK_EQUAL( coordinates.var_type(0) , CField2::VECTOR_2D );
  BOOST_CHECK_EQUAL( coordinates.var_type("coordinates") , CField2::VECTOR_2D );
  BOOST_CHECK_EQUAL( coordinates.topology().full_path().string() , mesh.topology().full_path().string() );
  BOOST_CHECK_EQUAL( coordinates.space_idx() , 0u );
  
  
  CNodes& nodes = mesh.nodes();  
  index_foreach(data_idx, const Uint node_idx, coordinates.used_nodes().array())
    coordinates[data_idx] = nodes.coordinates()[node_idx];
  
  BOOST_CHECK( coordinates.data().array() == nodes.coordinates().array() );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( SolutionFieldCreation )
{
	CMesh& mesh = *m_mesh;

  std::vector<std::string> names;
  std::vector<std::string> types;

  CField2& solution = *mesh.create_component<CField2>("solution");
  names = list_of("rho")("U")("p");
  types = list_of("scalar")("Vector2D")("scalar");
  solution.configure_property("Topology",mesh.topology().full_path());
  solution.configure_property("VarNames",names);
  solution.configure_property("VarTypes",types);
  solution.configure_property("FieldType",std::string("PointBased"));
  solution.create_data_storage();
  
  
  BOOST_CHECK_EQUAL( solution.basis() , CField2::Basis::POINT_BASED );
  BOOST_CHECK_EQUAL( solution.var_name() , std::string("rho") );
  BOOST_CHECK_EQUAL( solution.var_name(0) , std::string("rho") );
  BOOST_CHECK_EQUAL( solution.var_name(1) , std::string("U") );
  BOOST_CHECK_EQUAL( solution.var_name(2) , std::string("p") );
  BOOST_CHECK_EQUAL( solution.var_number("rho") , 0u );
  BOOST_CHECK_EQUAL( solution.var_number("U") , 1u );
  BOOST_CHECK_EQUAL( solution.var_number("p") , 2u );
  BOOST_CHECK_EQUAL( solution.var_index("rho") , 0u );
  BOOST_CHECK_EQUAL( solution.var_index("U") , 1u );
  BOOST_CHECK_EQUAL( solution.var_index("p") , 3u );
  BOOST_CHECK_EQUAL( solution.var_type() , CField2::SCALAR );
  BOOST_CHECK_EQUAL( solution.var_type(0) , CField2::SCALAR );
  BOOST_CHECK_EQUAL( solution.var_type(1) , CField2::VECTOR_2D );
  BOOST_CHECK_EQUAL( solution.var_type(2) , CField2::SCALAR );
  BOOST_CHECK_EQUAL( solution.var_type("rho") , CField2::SCALAR );
  BOOST_CHECK_EQUAL( solution.var_type("U") , CField2::VECTOR_2D );
  BOOST_CHECK_EQUAL( solution.var_type("p") , CField2::SCALAR );
  BOOST_CHECK_EQUAL( solution.topology().full_path().string() , mesh.topology().full_path().string() );
  BOOST_CHECK_EQUAL( solution.space_idx() , 0u );
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldOperators )
{
  CMesh& mesh = *m_mesh;

  std::vector<std::string> names;
  std::vector<std::string> types;


  CField2& solution = *mesh.get_child_ptr("solution")->as_ptr<CField2>();
  CField2& solution_copy = *mesh.create_component<CField2>("solution_copy");
  names = list_of("rho")("U")("p");
  types = list_of("scalar")("Vector2D")("scalar");
  solution_copy.configure_property("Topology",mesh.topology().full_path());
  solution_copy.configure_property("VarNames",names);
  solution_copy.configure_property("VarTypes",types);
  solution_copy.configure_property("FieldType",std::string("PointBased"));
  solution_copy.create_data_storage();

  solution.data()[0][0] = 25.;
  solution_copy.data() = solution.data();
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 25. );
  solution_copy.data() += solution_copy.data();
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 50. );
  solution_copy.data() *= 2;
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 100. );
  solution_copy.data() /= 2;
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 50. );
  solution_copy.data() *= solution_copy.data();
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 2500. );
  solution_copy.data() /= solution_copy.data();
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 1. );
  solution_copy.data() -= solution_copy.data();
  BOOST_CHECK_EQUAL ( solution_copy.data()[0][0] , 0. );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

