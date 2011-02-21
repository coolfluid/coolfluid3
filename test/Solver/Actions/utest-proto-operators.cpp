// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CNodes.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"


#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace CF::Math::MathConsts;

using namespace boost;

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector< SF::Line1DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1
> HigherIntegrationElements;

typedef boost::mpl::vector< SF::Line1DLagrangeP1,
                            SF::Triag2DLagrangeP1,
                            SF::Quad2DLagrangeP1,
                            SF::Hexa3DLagrangeP1,
                            SF::Tetra3DLagrangeP1
> VolumeTypes;

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
  CMesh::Ptr mesh( allocate_component<CMesh>("rect") );
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);
  
  RealVector center_coords(2);
  center_coords.setZero();
  
  // Use the volume function
  for_each_element<VolumeTypes>(mesh->topology(), _cout << "Volume = " << volume << ", centroid = " << transpose(coordinates(center_coords)) << "\n");
  std::cout << std::endl; // Can't be in expression
  
  // volume calculation
  Real vol1 = 0.;
  for_each_element<VolumeTypes>(mesh->topology(), vol1 += volume);
  
  CFinfo << "Mesh volume: " << vol1 << CFendl;
  
  // For an all-quad mesh, this is the same... cool or what? TODO: restore this
//   Real vol2 = 0.;
//   for_each_element<VolumeTypes>
//   (
//     mesh->topology(),
//     vol2 += 0.5*((coordinates(2, 0) - coordinates(0, 0)) * (coordinates(3, 1) - coordinates(1, 1))
//               -  (coordinates(2, 1) - coordinates(0, 1)) * (coordinates(3, 0) - coordinates(1, 0)))
//   );
//   BOOST_CHECK_CLOSE(vol1, vol2, 1e-5);
}

// Deactivated, until for_each_element_node is ported from the old proto code
// BOOST_AUTO_TEST_CASE( VertexValence )
// {
//   // Create a 3x3 rectangle
//   CMesh::Ptr mesh( allocate_component<CMesh>("rect") );
//   Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 2, 2);
//   
//   // Set up a node-based field to store the number of cells that are adjacent to each node
//   const std::vector<std::string> vars(1, "Valence[1]");
//   mesh->create_field("Valences", vars, CField::NODE_BASED);
//   
//   // Set up proto variables
//   MeshTerm<0, ConstNodes> nodes( "Region", find_component_ptr_recursively_with_name<CRegion>(*mesh, "region") ); // Mesh geometry nodes
//   MeshTerm<1, Field<Real> > valence("Valences", "Valence"); // Valence field
//   
//   // Count the elements!
//   for_each_element<SF::VolumeTypes>(find_component_recursively_with_name<CRegion>(*mesh, "region")
//                                   , for_each_element_node(nodes, valence[_elem_node]++));
//   
//   // output the result
//   for_each_node(find_component_recursively_with_name<CRegion>(*mesh, "region")
//               , _cout << valence << " ");
//   std::cout << std::endl;
// }


BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;
  
  CMesh::Ptr mesh(allocate_component<CMesh>("circle"));
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);
  
  typedef boost::mpl::vector< SF::Line2DLagrangeP1> SurfaceTypes;
  
  RealVector2 force;
  force.setZero();
  
  for_each_element<SurfaceTypes>
  (
    mesh->topology(),
    force += integral<1>
    (
      pow<2>
      (
        2. * u * _sin( _atan2(coordinates[1], coordinates[0]) ) + circulation / (2. * pi() * radius)
      )  * 0.5 * rho * normal 
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
  
  CMesh::Ptr mesh = Core::instance().root()->create_component<CMesh>("circle");
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);
  
  mesh->create_scalar_field("Pressure", "p", CF::Mesh::CField2::Basis::POINT_BASED);
  
  CRegion::Ptr region = find_component_ptr_recursively_with_name<CRegion>(*mesh, "region");
  
  MeshTerm<1, Field<Real> > p("Pressure", "p"); // Pressure field

  typedef boost::mpl::vector< SF::Line2DLagrangeP1> SurfaceTypes;

  // Set a field with the pressures
  for_each_node
  (
    mesh->topology(),
    p += pow<2>
    (
      2. * u * _sin( _atan2(coordinates[1], coordinates[0]) ) + circulation / (2. * pi() * radius)
    )  * 0.5 * rho
  );

  MeshTerm<1, ConstField<Real> > const_p("Pressure", "p"); // Pressure field, const version
  
  RealVector2 force;
  force.setZero();
  
  for_each_element<SurfaceTypes>
  (
    mesh->topology(),
    force += integral<1>(const_p * normal)
  );
    

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}


// Must be run  before the next tests
BOOST_FIXTURE_TEST_CASE( CreateMesh2D, ProtoOperatorsFixture )
{
  ProtoOperatorsFixture::grid_2d = allocate_component<CMesh>("grid_2d");
  Tools::MeshGeneration::create_rectangle(*grid_2d, 1., 1., 2000, 2000);
}


/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect2D, ProtoOperatorsFixture ) // timed and profiled
{
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, find_components_recursively<CElements>(*grid_2d))
  {
    const CTable<Real>& coords = region.nodes().coordinates();
    const CTable<Uint>::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    SF::Quad2DLagrangeP1::NodeMatrixT nodes;
    for(Uint element = 0; element != element_count; ++element)
    {
      fill(nodes, coords, ctbl[element]);
      volume += (nodes(2, XX) - nodes(0, XX)) * (nodes(3, YY) - nodes(1, YY)) -
          (nodes(2, YY) - nodes(0, YY)) * (nodes(3, XX) - nodes(1, XX));
    }
  }
  volume *= 0.5;
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  
  for_each_element<SF::CellTypes>(grid_2d->topology(), vol += volume);
  
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral2D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  
  for_each_element<SF::CellTypes>(grid_2d->topology(), vol += integral<1>(jacobian_determinant));
  
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

// Compute volume through 4th order integration
BOOST_FIXTURE_TEST_CASE( IntegralOrder4, ProtoOperatorsFixture )
{
  Real vol = 0.;
  
  for_each_element<HigherIntegrationElements>(grid_2d->topology(), vol += integral<4>(jacobian_determinant));
  
  BOOST_CHECK_CLOSE(vol, 1., 0.0001);
}

BOOST_FIXTURE_TEST_CASE( CreateMesh3D, ProtoOperatorsFixture )
{
  channel_3d = allocate_component<CMesh>("channel_3d");
  BlockMesh::BlockData block_data;
  Tools::MeshGeneration::create_channel_3d(block_data, 10., 0.5, 5., 160, 80, 120, 0.1); // 160, 80, 120
  std::vector<Uint> nodes_dist;
  BlockMesh::build_mesh(block_data, *channel_3d, nodes_dist);
}

// Compute volume
BOOST_FIXTURE_TEST_CASE( Volume3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  
  for_each_element<SF::CellTypes>(channel_3d->topology(), vol += volume);
  
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

// Compute volume through integration
BOOST_FIXTURE_TEST_CASE( Integral3D, ProtoOperatorsFixture )
{
  Real vol = 0.;
  
  for_each_element<SF::CellTypes>(channel_3d->topology(), vol += integral<1>(jacobian_determinant));
  
  BOOST_CHECK_CLOSE(vol, 50., 1e-6);
}

/// Non-proto calculation, as reference
BOOST_FIXTURE_TEST_CASE( VolumeDirect3D, ProtoOperatorsFixture ) // timed and profiled
{
  const CElements& elems = find_component_recursively_with_name<CElements>(*channel_3d, "elements_CF.Mesh.SF.Hexa3DLagrangeP1");
  const CTable<Real>& coords = elems.nodes().coordinates();
  const CTable<Uint>::ArrayT conn = elems.connectivity_table().array();
  const Uint nb_elems = conn.size();
  Real volume = 0.0;
  SF::Hexa3DLagrangeP1::NodeMatrixT nodes;
  for(Uint elem = 0; elem != nb_elems; ++elem)
  {
    fill(nodes, coords, conn[elem]);
    volume += SF::Hexa3DLagrangeP1::volume(nodes);
  }
  BOOST_CHECK_CLOSE(volume, 50., 1e-6);
}

BOOST_FIXTURE_TEST_CASE( SurfaceIntegral3D, ProtoOperatorsFixture )
{
  RealVector3 area;
  area.setZero();
  
  CRegion& region = find_component_recursively_with_name<CRegion>(*channel_3d, "bottomWall");
  
  for_each_element< boost::mpl::vector<SF::Quad3DLagrangeP1> >
  (
    region,
    area += integral<1>(normal)
  );
  
  /// Normal vector on the bottom wall should point down, with a length equal to the area
  BOOST_CHECK_SMALL(area[XX], 1e-10);
  BOOST_CHECK_CLOSE(area[YY], -50., 1e-8);
  BOOST_CHECK_SMALL(area[ZZ], 1e-10);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
