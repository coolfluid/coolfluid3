// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::tecplot::Writer"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/OptionList.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionURI.hpp"
#include "mesh/MeshWriter.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( VTKXMLSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( WriteGrid )
{
  Component& root = Core::instance().root();

  Handle<Mesh> mesh = root.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);

  boost::shared_ptr< MeshWriter > vtk_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.VTKXML.Writer","meshwriter");

  std::vector<URI> fields; fields.push_back(mesh->geometry_fields().coordinates().uri());
  vtk_writer->options().configure_option("fields",fields);
  vtk_writer->options().configure_option("mesh",mesh);
  vtk_writer->options().configure_option("file",URI("grid.vtu"));
  vtk_writer->execute();

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

