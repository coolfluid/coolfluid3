// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Tecplot::CWriter"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VTKXMLSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteGrid )
{
  CRoot& root = Core::instance().root();

  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);

  CMeshWriter::Ptr vtk_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter","meshwriter");
  
  std::vector<Field::Ptr> fields; fields.push_back(mesh->geometry().coordinates().as_ptr<Field>());
  vtk_writer->set_fields(fields);
  
  vtk_writer->write_from_to(*mesh,"grid.vtu");

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

