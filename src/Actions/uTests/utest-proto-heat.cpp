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
#include "Actions/Proto/ProtoElementData.hpp"

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
  MeshTerm<1, EigenDenseMatrix<RealMatrix> > blocks(M);
  
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

BOOST_AUTO_TEST_CASE( Heat1D )
{
  Real lenght     =     5.;
  Real temp_start =   100.;
  Real temp_stop  =   500.;

  const Uint nb_segments = 20;

  // build the mesh
  CMesh::Ptr mesh(new CMesh("line"));
  Tools::MeshGeneration::create_line(*mesh, lenght, nb_segments);
  
  // Build the system matrix
  MeshTerm<0, ConstNodes> nodes;
  RealMatrix M(nb_segments+1, nb_segments+1);
  M.setZero();
  MeshTerm<1, EigenDenseMatrix<RealMatrix> > blocks(M);
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    blocks += integral<1>(laplacian(nodes))
  );
  
  // Set boundary conditions
  RealVector rhs(nb_segments+1);
  rhs = RealVector::Constant(nb_segments+1, 1, 0.);
  MeshTerm<0, EigenDenseDirichletBC<RealMatrix, RealVector> > bc(M, rhs); // as long as we don't mix expressions, we can reuse index 0
  
  // Left boundary at 10 degrees
  for_each_node
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "xneg"),
    bc = temp_start
  );
  
  // Right boundary at 25 degrees
  for_each_node
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "xpos"),
    bc = temp_stop
  );
  
  // Solve the system!
  const RealVector solution = M.colPivHouseholderQr().solve(rhs);
  
  // Check solution
  for(int i = 0; i != solution.rows(); ++i)
  {
    Real x = i * lenght / static_cast<Real>(nb_segments);
    CFinfo << "T(" << x << ") = " << solution[i] << CFendl;
    BOOST_CHECK_CLOSE(solution[i],temp_start + i * ( temp_stop - temp_start ) / static_cast<Real>(nb_segments), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE( GrammarEval )
{
  // build the mesh
  CMesh::Ptr mesh(new CMesh("line"));
  const Uint nb_segments = 5;
  Tools::MeshGeneration::create_line(*mesh, 1., nb_segments);
  
  // Build the system matrix
  MeshTerm<0, ConstNodes> nodes( recursive_get_named_component_typed_ptr<CRegion>(*mesh, "region") );
  
  for_each_element_new< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    _cout << nodes << "\n"
  );
  
  std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
