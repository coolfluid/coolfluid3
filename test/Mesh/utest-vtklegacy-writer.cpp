// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Tecplot::CWriter"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VTKLegacySuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteGrid )
{
  CRoot& root = Core::instance().root();

  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);

  CMeshWriter::Ptr vtk_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter","meshwriter");
  vtk_writer->write_from_to(*mesh,"grid.vtk");

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

