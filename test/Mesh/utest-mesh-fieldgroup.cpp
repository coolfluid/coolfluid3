// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::FieldGroup"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Math/MatrixTypes.hpp"
#include "Math/VariablesDescriptor.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCells.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct FieldGroupTests_Fixture
{
  /// common setup for each test case
  FieldGroupTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~FieldGroupTests_Fixture()
  {
  }

  /// common mesh accessed by all tests
  static CMesh::Ptr m_mesh;
};

CMesh::Ptr FieldGroupTests_Fixture::m_mesh = Core::instance().root().create_component_ptr<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldGroupTests_TestSuite, FieldGroupTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_MeshCreation )
{
  CSimpleMeshGenerator::create_rectangle(*m_mesh,5.,5.,5u,5u);
//  Core::instance().root().add_component(m_mesh);
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldGroup )
{
  CMesh& mesh = *m_mesh;

  // Check if nodes field_group is sane
  BOOST_CHECK_NO_THROW(mesh.geometry().check_sanity());

  // Check if indexes_for_element function returns expected results
  boost_foreach(CElements& elements, mesh.geometry().elements_range())
  {
    for (Uint e=0; e<elements.size(); ++e)
    {
      BOOST_CHECK( mesh.geometry().indexes_for_element(elements,e) == elements.node_connectivity()[e] );
    }
  }
  BOOST_CHECK_EQUAL( mesh.geometry().elements_lookup().components().size() , 5u);


  // ----------------------------------------------------------------------------------------------
  // CHECK element-based field group building

  // Create space and field_group for Lagrange P0 elements
  boost_foreach(CElements& elements, find_components_recursively<CElements>(mesh.topology()))
    elements.create_space("elems_P0","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP0");
  FieldGroup& elem_fields = mesh.create_field_group("elems_P0", FieldGroup::Basis::ELEMENT_BASED);

  BOOST_CHECK_EQUAL( elem_fields.size() , 45);
  BOOST_CHECK_EQUAL( elem_fields.elements_lookup().components().size() , 5u);

  // Create space and field_group for Lagrange P0 cells
  boost_foreach(CCells& elements, find_components_recursively<CCells>(mesh.topology()))
    elements.create_space("cells_P0","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP0");
  FieldGroup& cell_fields = mesh.create_field_group("cells_P0", FieldGroup::Basis::CELL_BASED);

  BOOST_CHECK_EQUAL( cell_fields.size() , 25);
  BOOST_CHECK_EQUAL( cell_fields.elements_lookup().components().size() , 1u);

  // Create space and field_group for Lagrange P0 faces
  boost_foreach(CFaces& elements, find_components_recursively<CFaces>(mesh.topology()))
    elements.create_space("faces_P0","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP0");
  FieldGroup& face_fields = mesh.create_field_group("faces_P0", FieldGroup::Basis::FACE_BASED);

  BOOST_CHECK_EQUAL( face_fields.size() , 20);
  BOOST_CHECK_EQUAL( face_fields.elements_lookup().components().size() , 4u);

  // CHECK indexes_for_element access for nodes
  Uint cell_idx=0;
  boost_foreach(CEntities& elements, cell_fields.elements_range())
  {
    for (Uint e=0; e<elements.size(); ++e)
    {
      BOOST_CHECK( elements.space(cell_fields.space()).is_bound_to_fields() );
      boost_foreach( const Uint point, cell_fields.indexes_for_element(elements,e) )
      {
        BOOST_CHECK_EQUAL( point, cell_idx );  // same because P0 field
        ++cell_idx;
      }
    }
  }
  // Same check using a unified_cell_index
  for (Uint unified_idx=0; unified_idx<cell_fields.elements_lookup().size(); ++unified_idx)
  {
    boost_foreach( const Uint point, cell_fields.indexes_for_element(unified_idx) )
    {
      BOOST_CHECK_EQUAL( point, unified_idx);  // The boost_foreach has only 1 iteration as it is P0 field
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

  BOOST_CHECK_EQUAL(cell_fields.fields().size() , 1u);
  boost_foreach(Field& field, cell_fields.fields())
    BOOST_CHECK_EQUAL( field.field_group().uri().string() , cell_fields.uri().string());

  BOOST_CHECK_EQUAL(cell_fields.field("volume").uri().string() , volume.uri().string());

  // ----------------------------------------------------------------------------------------------
  // CHECK P1 point-based field group building

  // Create space for Lagrange P1 elements
  boost_foreach(CEntities& elements, find_components_recursively<CEntities>(mesh.topology()))
    elements.create_space("P1","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP1");

  // Create field group for the space "P1"
  FieldGroup& point_P1_fields = mesh.create_field_group("points_P1", FieldGroup::Basis::POINT_BASED, "P1");

  BOOST_CHECK_EQUAL ( point_P1_fields.size() , mesh.geometry().size() );

  // ----------------------------------------------------------------------------------------------
  // CHECK P2 point-based field group building

  // Create space for Lagrange P2 elements
  boost_foreach(CEntities& elements, find_components_recursively<CEntities>(mesh.topology()))
    elements.create_space("P2","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP2");

  // Create field group for the space "P2"
  FieldGroup& point_P2_fields = mesh.create_field_group("points_P2", FieldGroup::Basis::POINT_BASED, "P2");
  BOOST_CHECK_EQUAL ( point_P2_fields.size() , 121u );


//  std::cout << mesh.tree() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Field )
{
  FieldGroup& cells_P0 = m_mesh->get_child("cells_P0").as_type<FieldGroup>();
  Field& volume = cells_P0.field("volume");
  boost_foreach(CElements& elements, volume.elements_range())
  {
    CSpace& space = volume.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      boost_foreach( const Uint state, space.indexes_for_element(e))
      {
        volume[state][0] = elements.element_type().compute_volume(elements.get_coordinates(e));
      }


    }
  }

  FieldGroup& points_P2 = m_mesh->get_child("points_P2").as_type<FieldGroup>();
  Field& point_field = points_P2.create_field("point_field");


  boost_foreach(const CElements& elements, point_field.elements_range())
  {
    const CSpace& space = point_field.space(elements);
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
  FieldGroup& cells_P0 = m_mesh->get_child("cells_P0").as_type<FieldGroup>();
  Field& solution = cells_P0.create_field("solution","sol[1]");
  Field& solution_copy = cells_P0.create_field("solution_copy",solution.descriptor().description());
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

