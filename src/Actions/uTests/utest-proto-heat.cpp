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
  
  for_each_element< boost::mpl::vector<SF::Line1DLagrangeP1> >
  (
    recursive_get_named_component_typed<CRegion>(*mesh, "region"),
    _cout << "elem result:\n" << integral<1>
    (
      (
        transpose(mapped_gradient(nodes)) * transpose(jacobian_adjoint(nodes)) *
        jacobian_adjoint(nodes) * mapped_gradient(nodes)
      ) / jacobian_determinant(nodes)
    )
    << "\n"
  );
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
