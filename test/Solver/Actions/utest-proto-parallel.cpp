// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for benchmarking proto operators"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Common/PE/all_reduce.hpp"
#include "Common/PE/debug.hpp"
#include "Common/PE/Comm.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeP0/Hexa.hpp"
#include "Mesh/LagrangeP1/Hexa3D.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/Tags.hpp"

#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CComputeVolume.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;
using namespace cf3::Mesh;
using namespace cf3::common;

////////////////////////////////////////////////////

struct ProtoParallelFixture :
  //public Tools::Testing::ProfiledTestFixture,
  public Tools::Testing::TimedTestFixture
{
  ProtoParallelFixture() :
    root(Core::instance().root()),
    length(12.),
    half_height(0.5),
    width(6.)
  {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    cf_assert(argc == 4);
    x_segs = boost::lexical_cast<Uint>(argv[1]);
    y_segs = boost::lexical_cast<Uint>(argv[2]);
    z_segs = boost::lexical_cast<Uint>(argv[3]);
  }

  // Setup a model under root
  CModel& setup(const std::string& model_name)
  {
    CModel& model = Core::instance().root().create_component<CModel>(model_name);
    Physics::PhysModel& phys_model = model.create_physics("CF.Physics.DynamicModel");
    CDomain& dom = model.create_domain("Domain");
    CSolver& solver = model.create_solver("CF.Solver.CSimpleSolver");

    CMesh& mesh = dom.create_component<CMesh>("mesh");
    CMesh& serial_block_mesh = dom.create_component<CMesh>("serial_block_mesh"); // temporary mesh used for paralellization

    const Real ratio = 0.1;

    BlockMesh::BlockData& blocks = dom.create_component<BlockMesh::BlockData>("blocks");
    Tools::MeshGeneration::create_channel_3d(blocks, length, half_height, width, x_segs, y_segs/2, z_segs, ratio);

    BlockMesh::BlockData& parallel_blocks = dom.create_component<BlockMesh::BlockData>("parallel_blocks");
    BlockMesh::partition_blocks(blocks, PE::Comm::instance().size(), XX, parallel_blocks);

    BlockMesh::build_mesh(parallel_blocks, mesh);

    // Set up variables
    phys_model.variable_manager().create_descriptor("variables", "CellVolume, CellRank");

    // Create field
    boost_foreach(CEntities& elements, mesh.topology().elements_range())
    {
      elements.create_space("elems_P0","CF.Mesh.LagrangeP0."+elements.element_type().shape_name());
    }

    return model;
  }

  CRoot& root;
  const Real length;
  const Real half_height;
  const Real width;
  typedef boost::mpl::vector2<LagrangeP1::Hexa3D, LagrangeP0::Hexa> ElementsT;

  Uint x_segs;
  Uint y_segs;
  Uint z_segs;
};


BOOST_AUTO_TEST_SUITE( ProtoParallelSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Initialize )
{
  PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  //Common::PE::wait_for_debugger(1);
}

BOOST_FIXTURE_TEST_CASE( SetupNoOverlap, ProtoParallelFixture )
{
  const Real rank = static_cast<Real>(PE::Comm::instance().rank());

  CModel& model = setup("NoOverlap");
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();
  FieldGroup& elems_P0 = mesh.create_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED);
  model.solver().field_manager().create_field("variables", elems_P0);

  MeshTerm<0, ScalarField> V("CellVolume", "variables");
  MeshTerm<1, ScalarField> R("CellRank", "variables");

  model.solver()
  << create_proto_action
  (
    "ComputeVolumeAndRank",
    elements_expression
    (
      ElementsT(),
      group <<
      (
        V = volume,
        R = rank
      )
    )
  );

  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  model.solver().configure_option_recursively(Solver::Tags::regions(), root_regions);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( SimulateNoOverlap, ProtoParallelFixture )
{
  root.get_child("NoOverlap").as_type<CModel>().simulate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( SetupOverlap, ProtoParallelFixture )
{
  CModel& model = setup("Overlap");
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  const Real rank = static_cast<Real>(PE::Comm::instance().rank());

  MeshTerm<0, ScalarField> V("CellVolume", "variables");
  MeshTerm<1, ScalarField> R("CellRank", "variables");

  model.solver()
  << create_proto_action
  (
    "ComputeVolumeAndRank",
    elements_expression
    (
      ElementsT(),
      group <<
      (
        V = volume,
        R = rank
      )
    )
  );

  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  model.solver().configure_option_recursively(Solver::Tags::regions(), root_regions);
}

BOOST_FIXTURE_TEST_CASE( BuildGlobalConn, ProtoParallelFixture )
{
  CModel& model = root.get_child("Overlap").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  CMeshTransformer& global_conn = model.domain().create_component("CGlobalConnectivity", "CF.Mesh.Actions.CGlobalConnectivity").as_type<CMeshTransformer>();
  global_conn.transform(mesh);
}

BOOST_FIXTURE_TEST_CASE( GrowOverlap, ProtoParallelFixture )
{
  CModel& model = root.get_child("Overlap").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  CMeshTransformer& grow_overlap = model.domain().create_component("GrowOverlap", "CF.Mesh.Actions.GrowOverlap").as_type<CMeshTransformer>();
  grow_overlap.transform(mesh);
}

BOOST_FIXTURE_TEST_CASE( CreateOverlapFields, ProtoParallelFixture )
{
  CModel& model = root.get_child("Overlap").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  FieldGroup& elems_P0 = mesh.create_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED);
  model.solver().field_manager().create_field("variables", elems_P0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( SimulateOverlap, ProtoParallelFixture )
{
  root.get_child("Overlap").as_type<CModel>().simulate();
}

////////////////////////////////////////////////////////////////////////////////

// Check the volume results
BOOST_FIXTURE_TEST_CASE( CheckResultNoOverlap, ProtoParallelFixture )
{
  MeshTerm<0, ScalarField> V("CellVolume", "variables");

  const Real wanted_volume = width*length*half_height*2.;

  CMesh& mesh = find_component_recursively_with_name<CMesh>(root.get_child("NoOverlap"), "mesh");
  std::cout << "Checking volume for mesh " << mesh.uri().path() << std::endl;
  Real vol_check = 0;
  for_each_element< ElementsT >(mesh.topology(), vol_check += V);

  if(PE::Comm::instance().is_active())
  {
    Real total_volume_check;
    PE::all_reduce(PE::Comm::instance().communicator(), PE::plus(), &vol_check, 1, &total_volume_check);
    BOOST_CHECK_CLOSE(total_volume_check, wanted_volume, 1e-6);
  }

  CMeshWriter& writer = root.create_component("Writer", "CF.Mesh.VTKXML.CWriter").as_type<CMeshWriter>();
  std::vector<Field::Ptr> fields;
  fields.push_back(find_component_ptr_recursively_with_name<Field>(mesh, "variables"));
  writer.set_fields(fields);
  writer.write_from_to(mesh, URI("utest-proto-parallel_output-" + mesh.parent().parent().name() + ".pvtu"));
}

// Check the volume results
BOOST_FIXTURE_TEST_CASE( CheckResultOverlap, ProtoParallelFixture )
{
  const Uint nb_procs = PE::Comm::instance().size();
  MeshTerm<0, ScalarField> V("CellVolume", "variables");

  const Real wanted_volume = width*length*half_height*2.;
  std::cout << "wanted_volume: " << wanted_volume << ", nb_procs: " << nb_procs << ", x_segs: " << x_segs << std::endl;
  const Real wanted_volume_overlap = wanted_volume + (nb_procs-1)*2.*wanted_volume/x_segs;

  CMesh& mesh = find_component_recursively_with_name<CMesh>(root.get_child("Overlap"), "mesh");
  Real vol_check = 0;
  for_each_element< ElementsT >(mesh.topology(), vol_check += V);

  if(PE::Comm::instance().is_active())
  {
    Real total_volume_check;
    PE::all_reduce(PE::Comm::instance().communicator(), PE::plus(), &vol_check, 1, &total_volume_check);
    BOOST_CHECK_CLOSE(total_volume_check, wanted_volume_overlap, 1e-6);
  }

  CMeshWriter& writer = root.create_component("Writer", "CF.Mesh.VTKXML.CWriter").as_type<CMeshWriter>();
  std::vector<Field::Ptr> fields;
  fields.push_back(find_component_ptr_recursively_with_name<Field>(mesh, "variables"));
  writer.set_fields(fields);
  writer.write_from_to(mesh, URI("utest-proto-parallel_output-" + mesh.parent().parent().name() + ".pvtu"));
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
