// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "SFDM/Core/CreateSpace.hpp"
#include "SFDM/Core/ShapeFunction.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::SFDM;
using namespace CF::SFDM::Core;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_SF )
{
  typedef SFDM::Core::ShapeFunction SFD_SF;
  CRoot& root = Common::Core::instance().root();
  SFD_SF& sol_line_p0 = root.build_component("sol_line_p0","CF.SFDM.SF.LineSolutionP0").as_type<SFD_SF>();
  SFD_SF& sol_line_p1 = root.build_component("sol_line_p1","CF.SFDM.SF.LineSolutionP1").as_type<SFD_SF>();
  SFD_SF& sol_line_p2 = root.build_component("sol_line_p2","CF.SFDM.SF.LineSolutionP2").as_type<SFD_SF>();

  SFD_SF& flx_line_p1 = root.build_component("flx_line_p1","CF.SFDM.SF.LineFluxP1").as_type<SFD_SF>();
  SFD_SF& flx_line_p2 = root.build_component("flx_line_p2","CF.SFDM.SF.LineFluxP2").as_type<SFD_SF>();
  SFD_SF& flx_line_p3 = root.build_component("flx_line_p3","CF.SFDM.SF.LineFluxP3").as_type<SFD_SF>();

  const Uint line0 = 0;

  SFD_SF::Points     all_pts   = flx_line_p2.points();
  SFD_SF::Lines      ksi_pts   = all_pts[KSI];
  SFD_SF::LinePoints line0_pts = ksi_pts[line0];

  BOOST_CHECK_EQUAL( all_pts.num_elements()   , 3u ); // 3 points total

  BOOST_CHECK_EQUAL( all_pts.size()   , 1u ); // 1 orientation KSI
  BOOST_CHECK_EQUAL( ksi_pts.size()   , 1u ); // 1 line in this orientation
  BOOST_CHECK_EQUAL( line0_pts.size() , 3u ); // 3 points in this line

  BOOST_CHECK_EQUAL( flx_line_p1.points()[KSI][line0][0] , 0u );
  BOOST_CHECK_EQUAL( flx_line_p1.points()[KSI][line0][1] , 1u );

  BOOST_CHECK_EQUAL( flx_line_p1.face_points()[KSI][line0][LEFT] , 0u );
  BOOST_CHECK_EQUAL( flx_line_p1.face_points()[KSI][line0][RIGHT], 1u );

  BOOST_CHECK_EQUAL( flx_line_p2.points()[KSI][line0][0] , 0u );
  BOOST_CHECK_EQUAL( flx_line_p2.points()[KSI][line0][1] , 1u );
  BOOST_CHECK_EQUAL( flx_line_p2.points()[KSI][line0][2] , 2u );

  BOOST_CHECK_EQUAL( flx_line_p2.face_points()[KSI][line0][LEFT] , 0u );
  BOOST_CHECK_EQUAL( flx_line_p2.face_points()[KSI][line0][RIGHT], 2u );

  BOOST_CHECK_EQUAL( flx_line_p3.points()[KSI][line0][0] , 0u );
  BOOST_CHECK_EQUAL( flx_line_p3.points()[KSI][line0][1] , 1u );
  BOOST_CHECK_EQUAL( flx_line_p3.points()[KSI][line0][2] , 2u );
  BOOST_CHECK_EQUAL( flx_line_p3.points()[KSI][line0][3] , 3u );

  BOOST_CHECK_EQUAL( flx_line_p3.face_points()[KSI][line0][LEFT] , 0u);
  BOOST_CHECK_EQUAL( flx_line_p3.face_points()[KSI][line0][RIGHT], 3u);

}

BOOST_AUTO_TEST_CASE( LineP2 )
{
  /// Create a mesh consisting of a line with length 1. and 20 divisions
  CMesh::Ptr mesh = Common::Core::instance().root().create_component_ptr<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(*mesh, 1., 20);


  /// Create a "space" for SFDM solution of order P2, and for flux a space of order P3
  SFDM::Core::CreateSpace::Ptr space_creator = allocate_component<SFDM::Core::CreateSpace>("space_creator");
  space_creator->configure_property("P",2u);
  space_creator->transform(*mesh);

  /// create the solution field with space "solution"
  CField& solution = mesh->create_field("solution_field",CField::Basis::CELL_BASED,"solution","var[scalar]");

  /// create the flux field (just for show) with space "flux"
  CField& flux = mesh->create_field("flux_field",CField::Basis::CELL_BASED,"flux","var[scalar]");

  /// Output some stuff
  CFinfo << mesh->tree() << CFendl;
  CFinfo << "elements = " << mesh->topology().recursive_elements_count() << CFendl;
  CFinfo << "solution_fieldsize = " << solution.size() << CFendl;

  /// Initialize solution field with the function sin(2*pi*x)
  Actions::CInitFieldFunction::Ptr init_field = Common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
  init_field->configure_property("Functions",std::vector<std::string>(1,"sin(2*pi*x)"));
  init_field->configure_property("Field",solution.full_path());
  init_field->transform(*mesh);

  CFinfo << "initialized solution field with data:\n" << solution.data() << CFendl;

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  boost::filesystem::path filename ("line.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,solution.as_ptr<CField>()));
  gmsh_writer->write_from_to(mesh,filename);

  CFinfo << "Mesh \"line.msh\" written" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

