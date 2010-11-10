// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Actions/Proto/ProtoElementLooper.hpp"
#include "Actions/Proto/ProtoNodeLooper.hpp"

#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"

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

using namespace CF;
using namespace CF::Actions;
using namespace CF::Actions::Proto;
using namespace CF::Mesh;
using namespace CF::Common;

using namespace boost;

BOOST_AUTO_TEST_SUITE( ProtoHeatSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Laplacian1D )
{
  CMesh::Ptr mesh(new CMesh("line"));
  Tools::MeshGeneration::create_line(*mesh, 5., 5);
  
  MeshTerm<0, ConstNodes> nodes;
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    _cout << "elem result:\n" << integral<1>(laplacian(nodes)) << "\n"
  );
  
  RealMatrix M(6, 6);
  M.setZero();
  BlockMatrix<RealMatrix> blocks(M);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    blocks += integral<1>
    (
      (
        transpose(mapped_gradient(nodes)) * transpose(jacobian_adjoint(nodes)) *
        jacobian_adjoint(nodes) * mapped_gradient(nodes)
      ) / jacobian_determinant(nodes)
    )
  );
  
  std::cout << M << std::endl;
  
}

BOOST_AUTO_TEST_CASE( RegionNodes )
{
  // build the mesh
  CMesh::Ptr mesh(new CMesh("line"));
  const Uint nb_segments = 5;
  Tools::MeshGeneration::create_line(*mesh, 5., nb_segments);
  
  // Build the system matrix
  MeshTerm<0, ConstNodes> nodes;
  RealMatrix M(nb_segments+1, nb_segments+1);
  M.setZero();
  BlockMatrix<RealMatrix> blocks(M);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    blocks += integral<1>(laplacian(nodes))
  );
  
  // Set boundary conditions
  RealVector rhs(nb_segments+1);
  rhs = RealVector::Constant(nb_segments+1, 1, 0.);
  DirichletBC<RealMatrix, RealVector> bc(M, rhs);
  
  // Left boundary at 10 degrees
  for_each_node
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "xneg"),
    bc = 10.
  );
  
  // Right boundary at 25 degrees
  for_each_node
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "xpos"),
    bc = 35.
  );
  
  // Solve the system!
  const RealVector solution = M.colPivHouseholderQr().solve(rhs);
  
  // Check solution
  for(int i = 0; i != solution.rows(); ++i)
  {
    Real x = i * 5. / static_cast<Real>(nb_segments);
    CFinfo << "T(" << x << ") = " << solution[i] << CFendl;
    BOOST_CHECK_CLOSE(solution[i], 10. + i * 25. / static_cast<Real>(nb_segments), 1e-6);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
