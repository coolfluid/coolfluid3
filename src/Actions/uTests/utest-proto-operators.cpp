// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Actions/ProtoElementLooper.hpp"
#include "Actions/ProtoNodeLooper.hpp"

#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"


#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace CF;
using namespace CF::Actions;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector< SF::Line1DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1
> HigherIntegrationElements;

struct ProtoOperatorsFixture : public //Tools::Testing::ProfiledTestFixture,
                                      Tools::Testing::TimedTestFixture
{
  static CMesh::Ptr grid_2d;
  static CMesh::Ptr channel_3d;
};

CMesh::Ptr ProtoOperatorsFixture::grid_2d;
CMesh::Ptr ProtoOperatorsFixture::channel_3d;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( ProtoBasics )
{
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  // Create the variables
  MeshTerm<0, ConstNodes> nodes;
  
  // Use the volume function
  for_each_element<SF::VolumeTypes>(*mesh, _cout << "Element " << _elem << ": volume = " << volume(nodes) << ", centroid = " << nodes(_mapped_coord) << "\n");
  std::cout << std::endl; // Can't be in expression
  
  // volume calculation
  Real vol1 = 0.;
  for_each_element<SF::VolumeTypes>(*mesh, vol1 += volume(nodes));
  
  CFinfo << "Mesh volume: " << vol1 << CFendl;

  // For an all-quad mesh, this is the same... cool or what?, also this doesn't work anymore
//   Real vol2 = 0.;
//   for_each_element<mpl::vector<SF::Quad2DLagrangeP1> >(*mesh, vol2 +=
//                    0.5*((nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY])
//                      -  (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX])));
//   BOOST_CHECK_CLOSE(vol1, vol2, 1e-5);
}

BOOST_AUTO_TEST_CASE( VertexValence )
{
  // Create a 3x3 rectangle
  CMesh::Ptr mesh(new CMesh("rect"));
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 2, 2);
  
  // Set up a node-based field to store the number of cells that are adjacent to each node
  const std::vector<std::string> vars(1, "Valence[1]");
  mesh->create_field("Valences", vars, CField::NODE_BASED);
  
  // Set up proto variables
  MeshTerm<0, ConstNodes> nodes; // Mesh geometry nodes
  MeshTerm<1, Field<Real> > valence("Valences", "Valence"); // Valence field
  
  // Count the elements!
  for_each_element<SF::VolumeTypes>(recursive_get_named_component_typed<CRegion>(*mesh, "region")
                                  , for_each_element_node(nodes, valence[_elem_node]++));
  
  // output the result
  for_each_node(recursive_get_named_component_typed<CRegion>(*mesh, "region")
              , _cout << valence << " ");
  std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;
  
  CMesh::Ptr mesh(new CMesh("circle"));
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);
  
  MeshTerm<0, ConstNodes> nodes;
  
  typedef boost::mpl::vector< SF::Line2DLagrangeP1> SurfaceTypes;
  
  RealVector force(0., 2);
  
  for_each_element<SurfaceTypes>(*mesh,
    force += integral<1>
      (
        pow<2>
        (
          2. * u * _sin(_atan_vec(nodes(_mapped_coord))) + circulation / (2. * Math::MathConsts::RealPi() * radius)
        )  * 0.5 * rho * normal(nodes) 
      )
  );

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}

/// First create a field with the pressure distribution, then integrate it
BOOST_AUTO_TEST_CASE( RotatingCylinderField )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;
  
  CMesh::Ptr mesh(new CMesh("circle"));
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);
  
  const std::vector<std::string> vars(1, "p[1]");
  mesh->create_field("Pressure", vars, CField::NODE_BASED);
  
  MeshTerm<0, ConstNodes> nodes;
  MeshTerm<1, Field<Real> > p("Pressure", "p"); // Pressure field

  typedef boost::mpl::vector< SF::Line2DLagrangeP1> SurfaceTypes;
  
  RealVector force(0., 2);
  for_each_node(recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    p = pow<2>
    (
      2. * u * _sin(_atan2(nodes[YY], nodes[XX])) + circulation / (2. * Math::MathConsts::RealPi() * radius)
    )  * 0.5 * rho);

  for_each_element<SurfaceTypes>(recursive_get_named_component_typed<CRegion>(*mesh, "region")
                               , force += integral<1>(p(_mapped_coord) * normal(nodes)));
    

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}


// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh2D, ProtoOperatorsFixture )
{
  ProtoOperatorsFixture::grid_2d.reset(new CMesh("grid_2d"));
  Tools::MeshGeneration::create_rectangle(*grid_2d, 1., 1., 2000, 2000);
}


/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect2D, ProtoOperatorsFixture ) // timed and profiled
{
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(*grid_2d))
  {
    const CArray& coords = region.coordinates();
    const CTable::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    ElementNodeValues<4, 2, 1> nodes;
    for(Uint element = 0; element != element_count; ++element)
    {
      nodes.fill(coords, ctbl[element]);
      volume += (nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY]) -
          (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX]);
    }
  }
  volume *= 0.5;
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume, using an unnecessarily high var index
BOOST_FIXTURE_TEST_CASE( VolumeVector10, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<9, ConstNodes> nodes; // setting this to 9 increases the overhead
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*grid_2d, vol += integral<1>(jacobian_determinant(nodes)));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through 4th order integration
BOOST_FIXTURE_TEST_CASE( IntegralOrder4, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<HigherIntegrationElements>(*grid_2d, vol += integral<4>(jacobian_determinant(nodes)));
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( CreateMesh3D, ProtoOperatorsFixture )
{
  channel_3d.reset(new CMesh("channel_3d"));
  BlockMesh::BlockData block_data;
  Tools::MeshGeneration::create_channel_3d(block_data, 10., 0.5, 5., 160, 80, 120, 0.1);
  std::vector<Uint> nodes_dist;
  BlockMesh::build_mesh(block_data, *channel_3d, nodes_dist);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*channel_3d, vol += volume(nodes));
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  MeshTerm<0, ConstNodes> nodes;
  for_each_element<SF::VolumeTypes>(*channel_3d, vol += integral<1>(jacobian_determinant(nodes)));
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect3D, ProtoOperatorsFixture ) // timed and profiled
{
  const CElements& elems = recursive_get_named_component_typed<CElements>(*channel_3d, "elements_Hexa3DLagrangeP1");
  const CArray& coords = elems.coordinates();
  const CTable::ArrayT conn = elems.connectivity_table().array();
  const Uint nb_elems = conn.size();
  Real volume = 0.0;
  ElementNodeValues<8, 3> nodes;
  for(Uint elem = 0; elem != nb_elems; ++elem)
  {
    nodes.fill(coords, conn[elem]);
    volume += SF::Hexa3DLagrangeP1::volume(nodes);
  }
  BOOST_CHECK_CLOSE(volume, 50., 1e-6);
}

BOOST_FIXTURE_TEST_CASE( SurfaceIntegral3D, ProtoOperatorsFixture )
{
  RealVector area(0., 3);
  MeshTerm<0, ConstNodes> nodes;
  for_each_element< boost::mpl::vector<SF::Quad3DLagrangeP1> >(recursive_get_named_component_typed<CRegion>(*channel_3d, "bottomWall")
                                                             , area += integral<1>(normal(nodes)));
  
  /// Normal vector on the bottom wall should point down, with a length equal to the area
  BOOST_CHECK_SMALL(area[XX], 1e-10);
  BOOST_CHECK_CLOSE(area[YY], -50., 1e-8);
  BOOST_CHECK_SMALL(area[ZZ], 1e-10);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
