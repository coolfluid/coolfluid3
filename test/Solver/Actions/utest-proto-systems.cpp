// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/mpl/max_element.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;

BOOST_AUTO_TEST_SUITE( ProtoSystemsSuite )

//Test using a fusion vector of proto expressions
BOOST_AUTO_TEST_CASE( SystemBasics )
{
  Real length              = 5.;
  const Uint nb_segments   = 5;
  
  // build the mesh
  CMesh::Ptr mesh = Core::instance().root()->create_component<CMesh>("line");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  Uint elem_idx = 0;
  
  mesh->create_scalar_field("Temperature", "T", CF::Mesh::CField2::Basis::POINT_BASED);
  
  MeshTerm<0, ConstField<Real> > temperature("Temperature", "T");
  
  RealVector mc(1);
  mc.setZero();
  
  for_each_element< boost::mpl::vector1<SF::Line1DLagrangeP1> >
  (
    mesh->topology(),
    group
    (
      _cout << 2. * integral<1>( laplacian(temperature) * laplacian(temperature) * laplacian(temperature) ) << "\n",
      _cout << "Volume for element " << elem_idx << ": " << volume << "\n",
      ++boost::proto::lit(elem_idx)
    )
  );
}

BOOST_AUTO_TEST_SUITE_END()
