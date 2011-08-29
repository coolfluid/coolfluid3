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

#include "Common/MPI/all_reduce.hpp"
#include "Common/MPI/PE.hpp"

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
#include "Mesh/SF/Types.hpp"
#include "Mesh/SF/SFHexaLagrangeP0.hpp"

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

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;

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
  }

  // Setup a model under root
  CModel& setup(const std::string& model_name)
  {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;

    cf_assert(argc == 4);
    const Uint x_segs = boost::lexical_cast<Uint>(argv[1]);
    const Uint y_segs = boost::lexical_cast<Uint>(argv[2]);
    const Uint z_segs = boost::lexical_cast<Uint>(argv[3]);

    CModel& model = Core::instance().root().create_component<CModel>(model_name);
    Physics::PhysModel& phys_model = model.create_physics("CF.Physics.DynamicModel");
    CDomain& dom = model.create_domain("Domain");
    CSolver& solver = model.create_solver("CF.Solver.CSimpleSolver");

    CMesh& mesh = dom.create_component<CMesh>("mesh");
    CMesh& serial_block_mesh = dom.create_component<CMesh>("serial_block_mesh"); // temporary mesh used for paralellization

    const Real ratio = 0.1;

    BlockMesh::BlockData blocks;
    Tools::MeshGeneration::create_channel_3d(blocks, length, half_height, width, x_segs, y_segs/2, z_segs, ratio);

    BlockMesh::BlockData parallel_blocks;
    BlockMesh::partition_blocks(blocks, serial_block_mesh, Comm::PE::instance().size(), XX, parallel_blocks);

    BlockMesh::build_mesh(parallel_blocks, mesh);

    // Set up variables
    phys_model.variable_manager().create_descriptor("variables", "CellVolume, CellRank, CellVolumeOverlap");

    // Create field
    boost_foreach(CEntities& elements, mesh.topology().elements_range())
    {
      elements.create_space("elems_P0","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP0");
    }
    FieldGroup& elems_P0 = mesh.create_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED);
    solver.field_manager().create_field("variables", elems_P0);

    return model;
  }

  CRoot& root;
  const Real length;
  const Real half_height;
  const Real width;
  typedef boost::mpl::vector2<SF::Hexa3DLagrangeP1, SF::SFHexaLagrangeP0> ElementsT;
};


BOOST_AUTO_TEST_SUITE( ProtoParallelSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Initialize )
{
  Comm::PE::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
}

BOOST_FIXTURE_TEST_CASE( SetupModel, ProtoParallelFixture )
{
  CModel& model = setup("ProtoModel");
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  const Real rank = static_cast<Real>(Comm::PE::instance().rank());
  
  MeshTerm<0, ScalarField> V("CellVolume", "variables");
  MeshTerm<0, ScalarField> Vo("CellVolumeOverlap", "variables");
  MeshTerm<0, ScalarField> R("CellRank", "variables");

  CMeshTransformer& grow_overlap = model.domain().create_component("GrowOverlap", "CF.Mesh.Actions.GrowOverlap").as_type<CMeshTransformer>();
  grow_overlap.set_mesh(mesh);
  
  model.solver()
    << create_proto_action("ComputeVolume", elements_expression(ElementsT(), V = volume))
    //<< grow_overlap
    << create_proto_action("ComputeVolumeOverlap", elements_expression(ElementsT(), Vo = volume))
    << create_proto_action("ComputeRank", elements_expression(ElementsT(), R = rank));

  std::vector<URI> root_regions;
  root_regions.push_back(mesh.topology().uri());
  model.solver().configure_option_recursively(Solver::Tags::regions(), root_regions);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_CASE( SimulateModel, ProtoParallelFixture )
{
  root.get_child("ProtoModel").as_type<CModel>().simulate();
}

////////////////////////////////////////////////////////////////////////////////

// Check the volume result
BOOST_FIXTURE_TEST_CASE( CheckResult, ProtoParallelFixture )
{
  MeshTerm<0, ScalarField> V("CellVolume", "variables");

  const Real wanted_volume = width*length*half_height*2.;

  BOOST_FOREACH(CMesh& mesh, find_components_recursively_with_name<CMesh>(root, "mesh"))
  {
    std::cout << "Checking volume for mesh " << mesh.uri().path() << std::endl;
    Real vol_check = 0;
    for_each_element< ElementsT >(mesh.topology(), vol_check += V);

    if(Comm::PE::instance().is_active())
    {
      Real total_volume_check;
      Comm::all_reduce(Comm::PE::instance().communicator(), Comm::plus(), &vol_check, 1, &total_volume_check);
      BOOST_CHECK_CLOSE(total_volume_check, wanted_volume, 1e-6);
    }
    
    CMeshWriter& writer = root.create_component("Writer", "CF.Mesh.VTKXML.CWriter").as_type<CMeshWriter>();
    std::vector<Field::Ptr> fields;
    fields.push_back(find_component_ptr_recursively_with_name<Field>(mesh, "variables"));
    writer.set_fields(fields);
    writer.write_from_to(mesh, URI("utest-proto-parallel_output.pvtu"));
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
