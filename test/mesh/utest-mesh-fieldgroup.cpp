// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::SpaceFields"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"

#include "math/MatrixTypes.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Cells.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct SpaceFieldsTests_Fixture
{
  /// common setup for each test case
  SpaceFieldsTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~SpaceFieldsTests_Fixture()
  {
  }

  /// common mesh accessed by all tests
  static Handle< Mesh > m_mesh;
};

Handle< Mesh > SpaceFieldsTests_Fixture::m_mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( SpaceFieldsTests_TestSuite, SpaceFieldsTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_MeshCreation )
{
  SimpleMeshGenerator& mesh_gen = *Core::instance().root().create_component<SimpleMeshGenerator>("mesh_gen");
  mesh_gen.options().configure_option("mesh",m_mesh->uri());
  mesh_gen.options().configure_option("lengths",std::vector<Real>(2,5.));
  mesh_gen.options().configure_option("nb_cells",std::vector<Uint>(2,5u));
  mesh_gen.execute();
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_SpaceFields )
{
  Mesh& mesh = *m_mesh;

  // Check if nodes field_group is sane
  BOOST_CHECK_NO_THROW(mesh.geometry_fields().check_sanity());

  // Check if indexes_for_element function returns expected results
  boost_foreach(const Handle<Elements>& elements_handle, mesh.geometry_fields().elements_range())
  {
    Elements& elements = *elements_handle;
    for (Uint e=0; e<elements.size(); ++e)
    {
      BOOST_CHECK( mesh.geometry_fields().space(elements).connectivity()[e] == elements.node_connectivity()[e] );
    }
  }
  BOOST_CHECK_EQUAL( mesh.geometry_fields().elements_lookup().components().size() , 5u);


  // ----------------------------------------------------------------------------------------------
  // CHECK element-based field group building

  // Create space and field_group for Lagrange P0 elements
  SpaceFields& elem_fields = mesh.create_space_and_field_group("elems_P0", SpaceFields::Basis::ELEMENT_BASED,"cf3.mesh.LagrangeP0");

  BOOST_CHECK_EQUAL( elem_fields.size() , 45);
  BOOST_CHECK_EQUAL( elem_fields.elements_lookup().components().size() , 5u);

  // Create space and field_group for Lagrange P0 cells
  SpaceFields& cell_fields = mesh.create_space_and_field_group("cells_P0", SpaceFields::Basis::CELL_BASED,"cf3.mesh.LagrangeP0");

  BOOST_CHECK_EQUAL( cell_fields.size() , 25);
  BOOST_CHECK_EQUAL( cell_fields.elements_lookup().components().size() , 1u);

  // Create space and field_group for Lagrange P0 faces
  SpaceFields& face_fields = mesh.create_space_and_field_group("faces_P0", SpaceFields::Basis::FACE_BASED,"cf3.mesh.LagrangeP0");

  BOOST_CHECK_EQUAL( face_fields.size() , 20);
  BOOST_CHECK_EQUAL( face_fields.elements_lookup().components().size() , 4u);

  // CHECK indexes_for_element access for nodes
  Uint cell_idx=0;
  boost_foreach(const Handle<Entities>& elements_handle, cell_fields.elements_range())
  {
    const Entities& elements = *elements_handle;
    const Space& space = cell_fields.space(elements);
    const Connectivity& field_connectivity = space.connectivity();
    for (Uint e=0; e<elements.size(); ++e)
    {
      BOOST_CHECK( space.is_bound_to_fields() );
      boost_foreach( const Uint point, field_connectivity[e] )
      {
        BOOST_CHECK_EQUAL( point, cell_idx );  // same because P0 field
        ++cell_idx;
      }
    }
  }

  // ----------------------------------------------------------------------------------------------
  // CHECK field building inside field groups

  Field& solution = elem_fields.create_field("solution","rho[s],V[v],p[s]");
  Field& volume   = cell_fields.create_field("volume");

  BOOST_CHECK_EQUAL(solution.size() , elem_fields.size());
  BOOST_CHECK_EQUAL(solution.row_size() , 4);
  BOOST_CHECK_EQUAL(solution.array().num_elements() , solution.size()*solution.row_size());
  BOOST_CHECK_EQUAL(solution.field_group().uri().string() , elem_fields.uri().string() );
  BOOST_CHECK_EQUAL(volume.size() , cell_fields.size());
  BOOST_CHECK_EQUAL(volume.field_group().uri().string() , cell_fields.uri().string() );

  // There should be 1 volume field and 1 coordinates field
  BOOST_CHECK_EQUAL(find_components<Field>(cell_fields).size() , 2u);
  boost_foreach(Field& field, find_components<Field>(cell_fields))
    BOOST_CHECK_EQUAL( field.field_group().uri().string() , cell_fields.uri().string());

  BOOST_CHECK_EQUAL(cell_fields.field("volume").uri().string() , volume.uri().string());

  // ----------------------------------------------------------------------------------------------
  // CHECK P1 point-based field group building

  // Create field group for the space "points_P1" fields
  SpaceFields& point_P1_fields = mesh.create_space_and_field_group("points_P1", SpaceFields::Basis::POINT_BASED, "cf3.mesh.LagrangeP1");

  BOOST_CHECK_EQUAL ( point_P1_fields.size() , mesh.geometry_fields().size() );

  // ----------------------------------------------------------------------------------------------
  // CHECK P2 point-based field group building

  // Create field group for the space "P2"
  SpaceFields& point_P2_fields = mesh.create_space_and_field_group("points_P2", SpaceFields::Basis::POINT_BASED, "cf3.mesh.LagrangeP2");
  BOOST_CHECK_EQUAL ( point_P2_fields.size() , 121u );


//  std::cout << mesh.tree() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Field )
{
  Handle<SpaceFields> cells_P0(m_mesh->get_child("cells_P0"));
  Field& volume = cells_P0->field("volume");
  boost_foreach(const Handle<Elements>& elements_handle, volume.elements_range())
  {
    Elements& elements = *elements_handle;
    Space& space = volume.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      boost_foreach( const Uint state, space.indexes_for_element(e))
      {
        volume[state][0] = elements.element_type().volume(elements.get_coordinates(e));
      }


    }
  }

  Handle<SpaceFields> points_P2(m_mesh->get_child("points_P2"));
  Field& point_field = points_P2->create_field("point_field");


  boost_foreach(const Handle<Elements>& elements_handle, point_field.elements_range())
  {
    Elements& elements = *elements_handle;
    const Space& space = point_field.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      boost_foreach( const Uint point, space.indexes_for_element(e) )
      point_field[point][0] = 1.;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( FieldOperators )
{
  Handle<SpaceFields> cells_P0(m_mesh->get_child("cells_P0"));
  Field& solution = cells_P0->create_field("solution","sol[1]");
  Field& solution_copy = cells_P0->create_field("solution_copy",solution.descriptor().description());
  solution_copy.descriptor().prefix_variable_names("copy_");

  solution[0][0] = 25.;
  solution_copy = solution;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 25. );
  solution_copy += solution_copy;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 50. );
  solution_copy *= 2;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 100. );
  solution_copy /= 2;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 50. );
  solution_copy *= solution_copy;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 2500. );
  solution_copy /= solution_copy;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 1. );
  solution_copy -= solution_copy;
  BOOST_CHECK_EQUAL ( solution_copy[0][0] , 0. );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

