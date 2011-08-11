// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BlockMesh2D )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Grid2D )
{
  CMeshWriter::Ptr writer =  build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKLegacy.CWriter", "writer");
  
  const Real length = 1.;
  const Real height = 1.;
  const Uint x_segs = 10;
  const Uint y_segs = 10;
  
  BlockMesh::BlockData blocks;
  
  blocks.scaling_factor = 1.;

  blocks.points += list_of(0.    )(0.    )(0.)
                 , list_of(length)(0.    )(0.)
                 , list_of(length)(height)(0.)
                 , list_of(0.    )(height)(0.)
                 , list_of(0.    )(0.    )(1.)
                 , list_of(length)(0.    )(1.)
                 , list_of(length)(height)(1.)
                 , list_of(0.    )(height)(1.);

  blocks.block_points += list_of(0)(1)(2)(3)(4)(5)(6)(7);
  blocks.block_subdivisions += list_of(x_segs)(y_segs)(1);
  blocks.block_gradings += list_of(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.)(1.);

  blocks.patch_names += "left", "right", "top",  "bottom", "front", "back";
  blocks.patch_types += "wall", "wall",  "wall", "wall",   "empty", "empty";
  blocks.patch_points += list_of(0)(4)(7)(3),
                         list_of(2)(6)(5)(1),
                         list_of(3)(7)(6)(2),
                         list_of(0)(1)(5)(4),
                         list_of(0)(3)(2)(1),
                         list_of(4)(5)(6)(7);

  blocks.block_distribution += 0, 1;
  
  CDomain& domain = Core::instance().root().create_component<CDomain>("domain");
  domain.add_component(writer);
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  
  BlockMesh::build_mesh(blocks, mesh);
  
  BOOST_CHECK_EQUAL(mesh.dimension(), 2);
  
  writer->write_from_to(mesh, URI("grid-2d.vtk"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

