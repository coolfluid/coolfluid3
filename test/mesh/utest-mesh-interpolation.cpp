// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Table.hpp"
#include "common/FindComponents.hpp"
#include "common/Link.hpp"

#include "math/MatrixTypesConversion.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Interpolator.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Field.hpp"

#include "mesh/PointInterpolator.hpp"


using namespace boost;
using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct MeshInterpolation_Fixture
{
  /// common setup for each test case
  MeshInterpolation_Fixture()
  {
     int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshInterpolation_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshInterpolation_TestSuite, MeshInterpolation_Fixture )

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level",(Uint)INFO);
  Core::instance().environment().options().set("exception_backtrace",true);
  Core::instance().environment().options().set("regist_signal_handlers",true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Interpolation )
{
  BOOST_CHECK( true );

  // create meshreader
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  BOOST_CHECK( true );

  Mesh& source = *Core::instance().root().create_component<Mesh>("box");
//  meshreader->read_mesh_into("../../resources/hextet.neu",source);

  boost::shared_ptr<MeshGenerator> mesh_gen = allocate_component<SimpleMeshGenerator>("meshgen");
  mesh_gen->options().set("nb_cells",std::vector<Uint>(3,5));
  mesh_gen->options().set("lengths",std::vector<Real>(3,10.));
  mesh_gen->options().set("mesh",source.uri());
  mesh_gen->execute();

  BOOST_CHECK_EQUAL( source.geometry_fields().coordinates().row_size() , (Uint)DIM_3D );

  Mesh& target = *Core::instance().root().create_component<Mesh>("quadtriag");
  meshreader->read_mesh_into("../../resources/quadtriag.neu",target);

  BOOST_CHECK_EQUAL( target.geometry_fields().coordinates().row_size() , (Uint)DIM_2D );

  //  boost::filesystem::path fp_target ("grid_c.cgns");
//	boost::shared_ptr< MeshReader > cgns_meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.CGNS.Reader","cgns_meshreader");
//  Handle< Mesh > target = cgns_meshreader->create_mesh_from(fp_target);


  // Create and configure interpolator.
  boost::shared_ptr< Interpolator > interpolator = allocate_component<Interpolator>("interpolator");
  interpolator->options().set("store",false);
  BOOST_CHECK(true);

  std::string nvars = "rho_n[1] , V_n[3] , p_n[1]";
  std::string nvars_2;
  std::string evars;
  std::string evars_2;

  nvars_2 = "rho_n_2[1] , V_n_2[3] , p_n_2[1]";
  evars =   "rho_e[1] , V_e[3] , p_e[1]";
  evars_2 = "rho_e_2[1] , V_e_2[3] , p_e_2[1]";

  Dictionary& source_elem_fields = source.create_discontinuous_space("elems_P1", "cf3.mesh.LagrangeP1");
  Dictionary& target_elem_fields = target.create_discontinuous_space("elems_P1", "cf3.mesh.LagrangeP1");
  Dictionary& source_node_fields = source.geometry_fields();
  Dictionary& target_node_fields = target.geometry_fields();

  // Create empty fields
  Field& s_nodebased   = source_node_fields.create_field( "nodebased",     "rho_n[1],   V_n[3],   p_n[1]" );
  Field& s_elembased   = source_elem_fields.create_field( "elementbased",  "rho_e[1],   V_e[3],   p_e[1]" );

  Field& t_nodebased   = target_node_fields.create_field( "nodebased",     "rho_n[1],   V_n[3],   p_n[1]"   );
  Field& t_nodebased_2 = target_node_fields.create_field( "nodebased_2",   "rho_n_2[1], V_n_2[3], p_n_2[1]" );
  Field& t_elembased   = target_elem_fields.create_field( "elementbased",  "rho_e[1],   V_e[3],   p_e[1]"   );

  BOOST_CHECK(true);

  for ( Uint idx=0; idx!=s_nodebased.size(); ++idx)
  {
    Field::ConstRow coords = s_nodebased.coordinates()[idx];

    Field::Row data = s_nodebased[idx];

    data[0]=coords[XX]+2.*coords[YY]+2.*coords[ZZ];
    data[1]=coords[XX];
    data[2]=coords[YY];
    data[3]=7.0;
    data[4]=coords[XX];

  }

  boost_foreach( const Handle<Entities>& s_elements, s_elembased.entities_range() )
  {
    const Space& space = s_elembased.space(*s_elements);
    const Field& coordinates = s_elembased.coordinates();
    for (Uint elem_idx = 0; elem_idx<s_elements->size(); ++elem_idx)
    {
      boost_foreach(const Uint pt, space.connectivity()[elem_idx])
      {
        s_elembased[pt][0]=coordinates[pt][XX]+2.*coordinates[pt][YY]+2.*coordinates[pt][ZZ];
        s_elembased[pt][1]=coordinates[pt][XX];
        s_elembased[pt][2]=coordinates[pt][YY];
        s_elembased[pt][3]=7.0;
        s_elembased[pt][4]=coordinates[pt][XX];
      }
    }
  }


  BOOST_CHECK(true);

  // Interpolate the source field data to the target field. Note it can be in same or different meshes
  BOOST_CHECK_NO_THROW(interpolator->interpolate(s_nodebased,s_elembased));
  BOOST_CHECK_NO_THROW(interpolator->interpolate(s_nodebased,t_nodebased));
  BOOST_CHECK_NO_THROW(interpolator->interpolate(s_elembased,t_nodebased_2));
  BOOST_CHECK_NO_THROW(interpolator->interpolate(s_elembased,t_elembased));

  BOOST_CHECK(true);

  // Write the fields to file.
  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");

  BOOST_CHECK(true);

  std::vector<URI> s_fields;
  boost_foreach(Field& field, find_components_recursively<Field>(source))
    s_fields.push_back(field.uri());

  meshwriter->options().set("fields",s_fields);
  meshwriter->options().set("file",URI("source.msh"));
  meshwriter->options().set("mesh",source.handle<Mesh>());
  meshwriter->execute();

  std::vector<URI> t_fields;
  boost_foreach(Field& field, find_components_recursively<Field>(target))
    t_fields.push_back(field.uri());

  meshwriter->options().set("fields",t_fields);
  meshwriter->options().set("file",URI("interpolated.msh"));
  meshwriter->options().set("mesh",target.handle<Mesh>());
  meshwriter->execute();
  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_new_interpolation )
{
  BOOST_CHECK( true );

  // Create mesh
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  Handle<Mesh> source_mesh = Core::instance().root().create_component<Mesh>("hextet_new");
//  meshreader->read_mesh_into("../../resources/hextet.neu",*source_mesh);
  boost::shared_ptr<MeshGenerator> mesh_gen = allocate_component<SimpleMeshGenerator>("meshgen");
  mesh_gen->options().set("nb_cells",std::vector<Uint>(3,5));
  mesh_gen->options().set("lengths",std::vector<Real>(3,10.));
  mesh_gen->options().set("mesh",source_mesh->uri());
  mesh_gen->execute();


  // Create and configure interpolator.
  boost::shared_ptr< PointInterpolator > point_interpolator = allocate_component<PointInterpolator>("interpolator");
//  interpolator->options().set("element_finder",std::string("cf3.mesh.Octtree"));
//  interpolator->options().set("stencil_computer",std::string("cf3.mesh.StencilComputerOcttree"));
//  interpolator->options().set("function",std::string("cf3.mesh.PseudoLaplacianLinearInterpolation"));


  point_interpolator->options().set("dict",source_mesh->geometry_fields().handle<Dictionary>());

  RealVector coord(3); coord << 5., 1., 1.;
  std::vector<Real> interpolated_value;
  const Field& source_field = source_mesh->geometry_fields().coordinates();
  point_interpolator->interpolate(source_field,coord,interpolated_value);
  CFinfo << "interpolated value = " << to_str(interpolated_value) << CFendl;

  CFinfo << point_interpolator->info() << CFendl;


  Dictionary& target_dict = source_mesh->create_continuous_space("target","cf3.mesh.LagrangeP1");
  Field& target_field = target_dict.create_field("target","target[vector]");

  boost::shared_ptr< AInterpolator > interpolator = allocate_component<Interpolator>("interpolator");
  interpolator->options().set("store",true);

  // first call: compute storage and store

  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh->geometry_fields().coordinates(),target_field));
  CFinfo << target_field << CFendl;

  // second call: use stored values
  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh->geometry_fields().coordinates(),target_field));
  CFinfo << target_field << CFendl;

  // turn off storage, and compute again on the fly
  interpolator->options().set("store",false);
  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh->geometry_fields().coordinates(),target_field));
  CFinfo << target_field << CFendl;


  Handle<Mesh> source_mesh_2 = Core::instance().root().create_component<Mesh>("quadtriag_new");
  meshreader->read_mesh_into("../../resources/quadtriag.neu",*source_mesh_2);

  Dictionary& target_dict_2 = source_mesh_2->create_continuous_space("target","cf3.mesh.LagrangeP2");
  Field& target_field_2 = target_dict_2.create_field("target","target[vector]");

  interpolator->options().set("store",true);

  // first call: compute storage and store
  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh_2->geometry_fields().coordinates(),target_field_2));
  CFinfo << target_field_2 << CFendl;

  // second call: use stored values
  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh_2->geometry_fields().coordinates(),target_field_2));
  CFinfo << target_field_2 << CFendl;

  // turn off storage, and compute again on the fly
  interpolator->options().set("store",false);
  BOOST_CHECK_NO_THROW(interpolator->interpolate(source_mesh_2->geometry_fields().coordinates(),target_field_2));
  CFinfo << target_field_2 << CFendl;

  target_field_2 = 0.;
  std::vector<Uint> source_vars = boost::assign::list_of(0)(1);
  std::vector<Uint> target_vars = boost::assign::list_of(1)(0);
  BOOST_CHECK_NO_THROW(interpolator->interpolate_vars(source_mesh_2->geometry_fields().coordinates(),target_field_2,source_vars,target_vars));
  CFinfo << target_field_2 << CFendl;

}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

