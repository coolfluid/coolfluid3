// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::Tecplot::Writer"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"

#include "mesh/MeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "mesh/List.hpp"
#include "mesh/Table.hpp"
#include "mesh/Geometry.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VTKXMLSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteGrid )
{
  Root& root = Core::instance().root();

  Mesh::Ptr mesh = root.create_component_ptr<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);

  MeshWriter::Ptr vtk_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.VTKXML.Writer","meshwriter");
  
  std::vector<Field::Ptr> fields; fields.push_back(mesh->geometry().coordinates().as_ptr<Field>());
  vtk_writer->set_fields(fields);
  
  vtk_writer->write_from_to(*mesh,"grid.vtu");

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

