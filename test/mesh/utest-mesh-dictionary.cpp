// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::Dictionary"

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
#include "mesh/Dictionary.hpp"
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

struct DictionaryTests_Fixture
{
  /// common setup for each test case
  DictionaryTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~DictionaryTests_Fixture()
  {
  }

  /// common mesh accessed by all tests
  static Handle< Mesh > m_mesh;
};

Handle< Mesh > DictionaryTests_Fixture::m_mesh = Core::instance().root().create_component<Mesh>("mesh");

template <typename Derived>
void map(Field& field, Eigen::Map<Derived>& v, const Uint row_idx, const Uint var_idx)
{
  new (&v) typename Derived::MapType( &field.array()[row_idx][field.descriptor().offset(var_idx)],field.descriptor().var_length(var_idx) );
}


template <typename Derived>
typename Derived::MapType map(Field& field, const Uint row_idx, const Uint var_idx)
{
  return typename Derived::MapType( &field.array()[row_idx][field.descriptor().offset(var_idx)],field.descriptor().var_length(var_idx) );
}


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( DictionaryTests_TestSuite, DictionaryTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_MeshCreation )
{
  SimpleMeshGenerator& mesh_gen = *Core::instance().root().create_component<SimpleMeshGenerator>("mesh_gen");
  mesh_gen.options().set("mesh",m_mesh->uri());
  mesh_gen.options().set("lengths",std::vector<Real>(2,5.));
  mesh_gen.options().set("nb_cells",std::vector<Uint>(2,5u));
  mesh_gen.execute();
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Dictionary )
{
  Mesh& mesh = *m_mesh;

  // Check if nodes dict is sane
  BOOST_CHECK_NO_THROW(mesh.geometry_fields().check_sanity());

  // Check if connectivity returns expected results
  boost_foreach(const Handle<Entities>& elements_handle, mesh.geometry_fields().entities_range())
  {
    Entities& elements = *elements_handle;
    for (Uint e=0; e<elements.size(); ++e)
    {
      BOOST_CHECK( mesh.geometry_fields().space(elements).connectivity()[e] == elements.geometry_space().connectivity()[e] );
    }
  }
  BOOST_CHECK_EQUAL( mesh.geometry_fields().entities_range().size() , 5u);


  // ----------------------------------------------------------------------------------------------
  // CHECK element-based field group building

  // Create space and dict for Lagrange P0 elements
  Dictionary& elem_fields = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");

  BOOST_CHECK_EQUAL( elem_fields.size() , 45);
  BOOST_CHECK_EQUAL( elem_fields.entities_range().size() , 5u);

  // CHECK connectivity access for nodes
  Uint cell_idx=0;
  boost_foreach(const Handle<Entities>& elements_handle, elem_fields.entities_range())
  {
    const Entities& elements = *elements_handle;
    const Space& space = elem_fields.space(elements);
    const Connectivity& field_connectivity = space.connectivity();
    for (Uint e=0; e<elements.size(); ++e)
    {
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
  Field& volume   = elem_fields.create_field("volume");

  BOOST_CHECK_EQUAL(solution.size() , elem_fields.size());
  BOOST_CHECK_EQUAL(solution.row_size() , 4);
  BOOST_CHECK_EQUAL(solution.array().num_elements() , solution.size()*solution.row_size());
  BOOST_CHECK_EQUAL(solution.dict().uri().string() , elem_fields.uri().string() );
  BOOST_CHECK_EQUAL(volume.size() , elem_fields.size());
  BOOST_CHECK_EQUAL(volume.dict().uri().string() , elem_fields.uri().string() );

  // There should be 1 coordinates field, 1 volume field, 1 solution field
  BOOST_CHECK_EQUAL(find_components<Field>(elem_fields).size() , 3u);
  boost_foreach(Field& field, find_components<Field>(elem_fields))
    BOOST_CHECK_EQUAL( field.dict().uri().string() , elem_fields.uri().string());

  BOOST_CHECK_EQUAL(elem_fields.field("volume").uri().string() , volume.uri().string());

  // ----------------------------------------------------------------------------------------------
  // CHECK P1 point-based field group building

  // Create field group for the space "points_P1" fields
  Dictionary& point_P1_fields = mesh.create_continuous_space("points_P1","cf3.mesh.LagrangeP1");

  BOOST_CHECK_EQUAL ( point_P1_fields.size() , mesh.geometry_fields().size() );

  // ----------------------------------------------------------------------------------------------
  // CHECK P2 point-based field group building

  // Create field group for the space "P2"
  Dictionary& point_P2_fields = mesh.create_continuous_space("points_P2","cf3.mesh.LagrangeP2");
  BOOST_CHECK_EQUAL ( point_P2_fields.size() , 121u );


//  std::cout << mesh.tree() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Field )
{
  Handle<Dictionary> elems_P0(m_mesh->get_child("elems_P0"));
  Field& volume = elems_P0->field("volume");
  boost_foreach(const Handle<Entities>& elements_handle, volume.entities_range())
  {
    Entities& elements = *elements_handle;
    const Space& space = volume.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      boost_foreach( const Uint state, space.connectivity()[e])
      {
        volume[state][0] = elements.element_type().volume(elements.geometry_space().get_coordinates(e));
      }


    }
  }

  Handle<Dictionary> points_P2(m_mesh->get_child("points_P2"));
  Field& point_field = points_P2->create_field("point_field");


  boost_foreach(const Handle<Entities>& elements_handle, point_field.entities_range())
  {
    Entities& elements = *elements_handle;
    const Space& space = point_field.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      boost_foreach( const Uint point, space.connectivity()[e] )
      point_field[point][0] = 1.;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldOperators )
{
  Handle<Dictionary> elems_P0(m_mesh->get_child("elems_P0"));
  Field& solution = elems_P0->create_field("solution","sol[1]");
  Field& solution_copy = elems_P0->create_field("solution_copy",solution.descriptor().description());
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

BOOST_AUTO_TEST_CASE( FieldVariables )
{
  Handle<Dictionary> elems_P0(m_mesh->get_child("elems_P0"));
  Field& solution = elems_P0->create_field("solution","p[s],U[v],T[s]");

  RealRowVector::MapType sol( &solution.array()[0][0], solution.row_size() );
  sol.setConstant(3.);

  RealRowVector2::MapType U(nullptr);  // or RealRowVector2::MapType U(nullptr,0);
  new (&U) RealRowVector2::MapType( &solution.array()[1][solution.var_offset(1)],solution.var_length(1) );
  U.setConstant(4.);

  BOOST_CHECK_EQUAL ( solution[0][0] , 3. );
  BOOST_CHECK_EQUAL ( solution[0][1] , 3. );
  BOOST_CHECK_EQUAL ( solution[0][2] , 3. );
  BOOST_CHECK_EQUAL ( solution[0][3] , 3. );
  BOOST_CHECK_EQUAL ( solution[1][0] , 0. );
  BOOST_CHECK_EQUAL ( solution[1][1] , 4. );
  BOOST_CHECK_EQUAL ( solution[1][2] , 4. );
  BOOST_CHECK_EQUAL ( solution[1][3] , 0. );
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

