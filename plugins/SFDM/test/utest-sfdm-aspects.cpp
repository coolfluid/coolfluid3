// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::SFDM"

#include <boost/test/unit_test.hpp>


#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Root.hpp"
#include "common/FindComponents.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/CField.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/Entities.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Domain.hpp"
#include "mesh/actions/CInitFieldFunction.hpp"
#include "mesh/actions/CreateSpaceP0.hpp"
#include "Solver/CModel.hpp"
#include "Solver/FlowSolver.hpp"
#include "Solver/CTime.hpp"
#include "SFDM/CreateSpace.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Reconstruct.hpp"
#include "SFDM/SFDWizard.hpp"

#include "mesh/SF/Quad2DLagrangeP1.hpp"
#include "Solver/Physics.hpp"
#include "Physics/src/Euler/Cons1D.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::mesh::SF;
using namespace cf3::common;
using namespace cf3::SFDM;
using namespace cf3::Solver;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_SF_lines )
{
  typedef SFDM::ShapeFunction SFD_SF;
  Root& root = Core::instance().root();
  SFD_SF& sol_line_p0 = root.create_component("sol_line_p0","CF.SFDM.SF.LineSolutionP0").as_type<SFD_SF>();
  SFD_SF& sol_line_p1 = root.create_component("sol_line_p1","CF.SFDM.SF.LineSolutionP1").as_type<SFD_SF>();
  SFD_SF& sol_line_p2 = root.create_component("sol_line_p2","CF.SFDM.SF.LineSolutionP2").as_type<SFD_SF>();

  SFD_SF& flx_line_p1 = root.create_component("flx_line_p1","CF.SFDM.SF.LineFluxP1").as_type<SFD_SF>();
  SFD_SF& flx_line_p2 = root.create_component("flx_line_p2","CF.SFDM.SF.LineFluxP2").as_type<SFD_SF>();
  SFD_SF& flx_line_p3 = root.create_component("flx_line_p3","CF.SFDM.SF.LineFluxP3").as_type<SFD_SF>();

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

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Reconstruction_lines )
{
  Root& root = common::Core::instance().root();
  Reconstruct& reconstruct = root.create_component("reconstruct","CF.SFDM.Reconstruct").as_type<Reconstruct>();
  std::vector<std::string> from_to(2);
  from_to[0] = "CF.SFDM.SF.LineSolutionP1";
  from_to[1] = "CF.SFDM.SF.LineFluxP2";
  reconstruct.configure_option("from_to",from_to);

  RealVector solution(2); solution << 0. , 4.;      // in cell <-1,1>
  RealVector rec_sol(3);  rec_sol  << 0. , 2. , 4.;
  RealVector rec_grad(3); rec_grad << 2. , 2. , 2.;
  BOOST_CHECK_EQUAL ( reconstruct.value(solution) ,        rec_sol  ) ;
  BOOST_CHECK_EQUAL ( reconstruct.gradient(solution,KSI) , rec_grad ) ;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_SF_quads )
{
  typedef SFDM::ShapeFunction SFD_SF;
  Root& root = Core::instance().root();
  SFD_SF& sol_quad_p0 = root.create_component("sol_quad_p0","CF.SFDM.SF.QuadSolutionP0").as_type<SFD_SF>();
  SFD_SF& sol_quad_p1 = root.create_component("sol_quad_p1","CF.SFDM.SF.QuadSolutionP1").as_type<SFD_SF>();
  SFD_SF& sol_quad_p2 = root.create_component("sol_quad_p2","CF.SFDM.SF.QuadSolutionP2").as_type<SFD_SF>();
  SFD_SF& flx_quad_p1 = root.create_component("flx_quad_p1","CF.SFDM.SF.QuadFluxP1").as_type<SFD_SF>();
  SFD_SF& flx_quad_p2 = root.create_component("flx_quad_p2","CF.SFDM.SF.QuadFluxP2").as_type<SFD_SF>();
  SFD_SF& flx_quad_p3 = root.create_component("flx_quad_p3","CF.SFDM.SF.QuadFluxP3").as_type<SFD_SF>();


  BOOST_CHECK_EQUAL( sol_quad_p0.nb_nodes() , 1u );
  BOOST_CHECK_EQUAL( sol_quad_p0.line().derived_type_name() , std::string("CF.SFDM.SF.LineSolutionP0") );

  BOOST_CHECK_EQUAL( sol_quad_p1.nb_nodes() , 4u );
  BOOST_CHECK_EQUAL( sol_quad_p1.line().derived_type_name() , std::string("CF.SFDM.SF.LineSolutionP1") );

  BOOST_CHECK_EQUAL( sol_quad_p2.nb_nodes() , 9u );
  BOOST_CHECK_EQUAL( sol_quad_p2.line().derived_type_name() , std::string("CF.SFDM.SF.LineSolutionP2") );

  BOOST_CHECK_EQUAL( flx_quad_p1.nb_nodes() , 4u );
  BOOST_CHECK_EQUAL( flx_quad_p1.line().derived_type_name() , std::string("CF.SFDM.SF.LineFluxP1") );

  BOOST_CHECK_EQUAL( flx_quad_p2.nb_nodes() , 9u );
  BOOST_CHECK_EQUAL( flx_quad_p2.line().derived_type_name() , std::string("CF.SFDM.SF.LineFluxP2") );

  BOOST_CHECK_EQUAL( flx_quad_p3.nb_nodes() , 20u );
  BOOST_CHECK_EQUAL( flx_quad_p3.line().derived_type_name() , std::string("CF.SFDM.SF.LineFluxP3") );

  const Uint line0 = 0;

  SFD_SF::Points     all_pts   = flx_quad_p1.points();
  SFD_SF::Lines      ksi_pts   = all_pts[KSI];
  SFD_SF::Lines      eta_pts   = all_pts[ETA];
  SFD_SF::LinePoints line0_pts = ksi_pts[line0];

  BOOST_CHECK_EQUAL( all_pts.num_elements()   , 4u ); // 4 points total
  BOOST_CHECK_EQUAL( all_pts.size()   , 2u ); // 2 orientations KSI,ETA
  BOOST_CHECK_EQUAL( ksi_pts.size()   , 1u ); // 1 line in this orientation
  BOOST_CHECK_EQUAL( eta_pts.size()   , 1u ); // 1 line in this orientation
  BOOST_CHECK_EQUAL( line0_pts.size() , 2u ); // 1 points in this line

  BOOST_CHECK_EQUAL( flx_quad_p1.points()[KSI][line0][0] , 0u );
  BOOST_CHECK_EQUAL( flx_quad_p1.points()[KSI][line0][1] , 1u );

  BOOST_CHECK_EQUAL( flx_quad_p1.face_points()[KSI][line0][LEFT] , 0u );
  BOOST_CHECK_EQUAL( flx_quad_p1.face_points()[KSI][line0][RIGHT], 1u );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Reconstruction_quads )
{
  Root& root = common::Core::instance().root();
  Reconstruct& reconstruct = root.create_component("reconstruct_quads","CF.SFDM.Reconstruct").as_type<Reconstruct>();
  std::vector<std::string> from_to(2);
  from_to[0] = "CF.SFDM.SF.QuadSolutionP0";
  from_to[1] = "CF.SFDM.SF.QuadFluxP1";
  reconstruct.configure_option("from_to",from_to);

  RealVector solution(1); solution << 2. ;      // in cell <-1,1><-1,1>
  RealVector rec_sol(4);  rec_sol  << 2. , 2. , 2. , 2.;
  RealVector rec_grad(4); rec_grad << 0. , 0. , 0. , 0.;
  BOOST_CHECK_EQUAL ( reconstruct.value(solution) ,        rec_sol  ) ;
  BOOST_CHECK_EQUAL ( reconstruct.gradient(solution,KSI) , rec_grad ) ;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_fields_lines )
{
  /// Create a mesh consisting of a line with length 1. and 20 divisions
  Mesh::Ptr mesh = common::Core::instance().root().create_component_ptr<Mesh>("mesh");
  SimpleMeshGenerator::create_line(*mesh, 1., 20);


  /// Create a "space" for SFDM solution of order P2, and for flux a space of order P3
  SFDM::CreateSpace::Ptr space_creator = allocate_component<SFDM::CreateSpace>("space_creator");
  space_creator->configure_option("P",2u);
  space_creator->transform(*mesh);

  /// create the solution field with space "solution"
  CField& solution = mesh->create_field("solution_field",CField::Basis::CELL_BASED,"solution","var[scalar]");

  /// create the flux field (just for show) with space "flux"
  CField& flux = mesh->create_field("flux_field",CField::Basis::CELL_BASED,"flux","var[scalar]");

  /// Output some stuff
  //CFinfo << mesh->tree() << CFendl;
  CFinfo << "elements = " << mesh->topology().recursive_elements_count() << CFendl;
  CFinfo << "solution_fieldsize = " << solution.size() << CFendl;

  /// Initialize solution field with the function sin(2*pi*x)
  Actions::CInitFieldFunction::Ptr init_field = common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
  init_field->configure_option("functions",std::vector<std::string>(1,"sin(2*pi*x)"));
  init_field->configure_option("field",solution.uri());
  init_field->transform(*mesh);

  //CFinfo << "initialized solution field with data:\n" << solution.data() << CFendl;

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.Writer","meshwriter");
  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,solution.as_ptr<CField>()));
  gmsh_writer->write_from_to(*mesh,URI("line.msh"));

  CFinfo << "Mesh \"line.msh\" written" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_fields_quads )
{
  /// Create a mesh consisting of a line with length 1. and 20 divisions
  Mesh::Ptr mesh = common::Core::instance().root().create_component_ptr<Mesh>("rectangle");
  SimpleMeshGenerator::create_rectangle(*mesh, 1., 1., 20, 20);


  /// Create a "space" for SFDM solution of order P2, and for flux a space of order P3
  SFDM::CreateSpace::Ptr space_creator = allocate_component<SFDM::CreateSpace>("space_creator");
  space_creator->configure_option("P",0u);
  space_creator->transform(*mesh);

  /// create the solution field with space "solution"
  CField& solution = mesh->create_field("solution_field",CField::Basis::CELL_BASED,"solution","var[scalar]");

  /// create the flux field (just for show) with space "flux"
  CField& flux = mesh->create_field("flux_field",CField::Basis::CELL_BASED,"flux","var[scalar]");

  /// Output some stuff
  //CFinfo << mesh->tree() << CFendl;
  CFinfo << "elements = " << mesh->topology().recursive_elements_count() << CFendl;
  CFinfo << "solution_fieldsize = " << solution.size() << CFendl;

  /// Initialize solution field with the function exp( -( (x-mu)^2+(y-mu)^2 )/(2*sigma^2) )
  Actions::CInitFieldFunction::Ptr init_field = common::Core::instance().root().create_component_ptr<Actions::CInitFieldFunction>("init_field");
  std::string gaussian="sigma:="+to_str(0.1)+"; mu:="+to_str(0.5)+"; exp( -( (x-mu)^2+(y-mu)^2 )/(2*sigma^2) )";
  init_field->configure_option("functions",std::vector<std::string>(1,gaussian));
  init_field->configure_option("field",solution.uri());
  init_field->transform(*mesh);

  //CFinfo << "initialized solution field with data:\n" << solution.data() << CFendl;

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.Writer","meshwriter");
  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,solution.as_ptr<CField>()));
  gmsh_writer->write_from_to(*mesh,URI("rectangle.msh"));

  CFinfo << "Mesh \"rectangle.msh\" written" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_mesh_transform )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions
  Mesh& mesh = common::Core::instance().root().get_child("rectangle").as_type<Mesh>();

  MeshTransformer& build_faces = common::Core::instance().root().create_component("build_faces","CF.Mesh.Actions.CBuildFaces").as_type<MeshTransformer>();
  build_faces.configure_option("store_cell2face",true);
  build_faces.transform(mesh);

  /// write gmsh file. note that gmsh gets really confused because of the multistate view
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.Writer","meshwriter");
//  gmsh_writer->set_fields(std::vector<CField::Ptr>(1,solution.as_ptr<CField>()));
  gmsh_writer->write_from_to(mesh,URI("rectangle.msh"));

  CFinfo << "Mesh \"rectangle.msh\" written" << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_computerhsincell_xdir )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions


  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("test"));
  wizard.configure_option("dim",2u);
  wizard.configure_option("RK_stages",1u);
  wizard.configure_option("solution_state",std::string("CF.AdvectionDiffusion.State2D"));
  wizard.configure_option("roe_state",std::string("CF.AdvectionDiffusion.State2D"));

  wizard.create_simulation();

  CModel& model = wizard.model();
  Mesh& mesh = model.domain().create_component<Mesh>("2quads");
  SimpleMeshGenerator::create_rectangle(mesh, 4., 2. , 2, 1);

  Component& iterate = model.solver().access_component("iterate");
  iterate.configure_option("max_iter",1u);

  wizard.prepare_simulation();


  std::string step="if(x<2,1.5,0.5)";

  CFinfo << "\nInitializing solution with [" << step << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,step));

  MeshWriter& gmsh_writer = model.tools().create_component("gmsh","CF.Mesh.Gmsh.Writer").as_type<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(mesh.get_child("solution").uri());
  fields.push_back(mesh.get_child("residual").uri());
  fields.push_back(mesh.get_child("wave_speed").uri());
  fields.push_back(mesh.get_child("update_coeff").uri());
  fields.push_back(mesh.get_child("volume").uri());
  fields.push_back(mesh.get_child("jacobian_determinant").uri());
  gmsh_writer.configure_option("fields",fields);


  gmsh_writer.write_from_to(mesh,URI("2quads_0.msh"));
  CFinfo << "Mesh \"2quads_0.msh\" written" << CFendl;

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("2quads_1.msh"));

  CFinfo << "Mesh \"2quads_1.msh\" written" << CFendl;

  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[0][0] , 0.);
  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[1][0] , 0.5);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[0][0] , 1.5);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[1][0] , 1.5);
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[0][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[1][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[0][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[1][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[0][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[1][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[0][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[1][0] , 2. );
  BOOST_CHECK_EQUAL (model.time().dt() , 2.);

}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( test_computerhsincell_ydir )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions


  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("test"));
  wizard.configure_option("dim",2u);
  wizard.configure_option("RK_stages",1u);

  wizard.create_simulation();

  CModel& model = wizard.model();
  Mesh& mesh = model.domain().create_component<Mesh>("2quads");
  SimpleMeshGenerator::create_rectangle(mesh, 2., 4. , 1, 2);

  Component& iterate = model.solver().access_component("iterate");
  iterate.configure_option("max_iter",1u);

  wizard.prepare_simulation();


  std::string step="if(y<2,1.5,0.5)";

  CFinfo << "\nInitializing solution with [" << step << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,step));

  MeshWriter& gmsh_writer = model.tools().create_component("gmsh","CF.Mesh.Gmsh.Writer").as_type<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(mesh.get_child("solution").uri());
  fields.push_back(mesh.get_child("residual").uri());
  fields.push_back(mesh.get_child("wave_speed").uri());
  fields.push_back(mesh.get_child("update_coeff").uri());
  fields.push_back(mesh.get_child("volume").uri());
  fields.push_back(mesh.get_child("jacobian_determinant").uri());
  gmsh_writer.configure_option("fields",fields);


  gmsh_writer.write_from_to(mesh,URI("2quads_y_0.msh"));
  CFinfo << "Mesh \"2quads_y_0.msh\" written" << CFendl;

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("2quads_y_1.msh"));

  CFinfo << "Mesh \"2quads_y_1.msh\" written" << CFendl;

  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[0][0] , 0.);
  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[1][0] , 0.5);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[0][0] , 1.5);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[1][0] , 1.5);
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[0][0] , 0. );
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[1][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[0][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[1][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[0][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[1][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[0][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[1][0] , 2. );
  BOOST_CHECK_EQUAL (model.time().dt() , 2.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_computerhsincell_xydir )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions


  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("test"));
  wizard.configure_option("dim",2u);
  wizard.configure_option("RK_stages",1u);

  wizard.create_simulation();

  CModel& model = wizard.model();
  Mesh& mesh = model.domain().create_component<Mesh>("2quads");
  SimpleMeshGenerator::create_rectangle(mesh, 4., 4. , 2, 2);

  Component& iterate = model.solver().access_component("iterate");
  iterate.configure_option("max_iter",1u);

  wizard.prepare_simulation();


  std::string step="if( x<2 & y<2 ,1.5,0.5)";

  CFinfo << "\nInitializing solution with [" << step << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,step));

  MeshWriter& gmsh_writer = model.tools().create_component("gmsh","CF.Mesh.Gmsh.Writer").as_type<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(mesh.get_child("solution").uri());
  fields.push_back(mesh.get_child("residual").uri());
  fields.push_back(mesh.get_child("wave_speed").uri());
  fields.push_back(mesh.get_child("update_coeff").uri());
  fields.push_back(mesh.get_child("volume").uri());
  fields.push_back(mesh.get_child("jacobian_determinant").uri());
  gmsh_writer.configure_option("fields",fields);


  gmsh_writer.write_from_to(mesh,URI("4quads_xy_0.msh"));
  CFinfo << "Mesh \"4quads_xy_0.msh\" written" << CFendl;

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("4quads_xy_1.msh"));

  CFinfo << "Mesh \"4quads__xy_1.msh\" written" << CFendl;

  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[0][0] , 0.);
  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[1][0] , 0.5);
  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[2][0] , 0.5);
  BOOST_CHECK_EQUAL (mesh.get_child("residual").as_type<CField>().data()[3][0] , 0.);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[0][0] , 1.5);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[1][0] , 1.0);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[2][0] , 1.0);
  BOOST_CHECK_EQUAL (mesh.get_child("solution").as_type<CField>().data()[3][0] , 0.5);
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[0][0] , 0. );
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[1][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[2][0] , 2. );
  BOOST_CHECK_EQUAL (mesh.get_child("wave_speed").as_type<CField>().data()[3][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[0][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[1][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[2][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("volume").as_type<CField>().data()[3][0] , 4. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[0][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[1][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[2][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("jacobian_determinant").as_type<CField>().data()[3][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[0][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[1][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[2][0] , 1. );
  BOOST_CHECK_EQUAL (mesh.get_child("update_coeff").as_type<CField>().data()[3][0] , 1. );
  BOOST_CHECK_EQUAL (model.time().dt() , 2.);

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("4quads_xy_2.msh"));

}
*/

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_riemannproblem_euler1D )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions


  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("test"));
  wizard.configure_option("dim",1u);
  wizard.configure_option("RK_stages",1u);
  wizard.configure_option("solution_state",std::string("CF.Euler.Cons1D"));
  wizard.configure_option("roe_state",std::string("CF.Euler.Roe1D"));
  wizard.create_simulation();

  CModel& model = wizard.model();
  Mesh& mesh = model.domain().create_component<Mesh>("2lines");
  const Real length = 4.;
  const Uint nb_cells = 4;
  SimpleMeshGenerator::create_line(mesh, length , nb_cells);

  Component& iterate = model.solver().access_component("iterate");
  iterate.configure_option("max_iter",2u);

  wizard.prepare_simulation();

  const Real r_L = 4.696;             const Real r_R = 1.408;
  const Real p_L = 404400;            const Real p_R = 101100;
  const Real u_L = 0.;                const Real u_R = 0.;
  const Real v_L = 0.;                const Real v_R = 0.;
  const Real g=1.4;

  RealVector3 left, right;
  left  << r_L , r_L*u_L , p_L/(g-1) + 0.5*r_L*u_L*u_L;
  right << r_R , r_R*u_R , p_R/(g-1) + 0.5*r_R*u_R*u_R;
  std::vector<std::string> function(3);
  for (Uint i=0; i<function.size(); ++i)
    function[i]="if(x<="+to_str(length/2.)+","+to_str(left[i])+","+to_str(right[i])+")";

  CFinfo << "\nInitializing solution with \n";
  boost_foreach(const std::string& f, function)
      CFinfo << "     " << f << CFendl;
  wizard.initialize_solution(function);

  MeshWriter& gmsh_writer = model.tools().create_component("gmsh","CF.Mesh.Gmsh.Writer").as_type<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(mesh.get_child("solution").uri());
  fields.push_back(mesh.get_child("residual").uri());
  fields.push_back(mesh.get_child("wave_speed").uri());
  fields.push_back(mesh.get_child("update_coeff").uri());
  fields.push_back(mesh.get_child("volume").uri());
  fields.push_back(mesh.get_child("jacobian_determinant").uri());
  gmsh_writer.configure_option("fields",fields);


  gmsh_writer.write_from_to(mesh,URI("2lines_0.msh"));
  CFinfo << "Mesh \"2lines_0.msh\" written" << CFendl;

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("2lines_1.msh"));

  CFinfo << "Mesh \"2lines_1.msh\" written" << CFendl;

//  BOOST_CHECK_EQUAL (model.time().dt() , 2.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_riemannproblem_euler2D )
{
  Core::instance().environment().configure_option("log_level", (Uint)DEBUG);
  /// Create a mesh consisting of a line with length 1. and 20 divisions


  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("test"));
  wizard.configure_option("dim",2u);
  wizard.configure_option("RK_stages",1u);
  wizard.configure_option("solution_state",std::string("CF.Euler.Cons2D"));
  wizard.configure_option("roe_state",std::string("CF.Euler.Roe2D"));
  wizard.create_simulation();

  CModel& model = wizard.model();
  Mesh& mesh = model.domain().create_component<Mesh>("2quads_euler");
  const Real length = 2.;
  const Uint nb_cells = 2;
  SimpleMeshGenerator::create_rectangle(mesh, length,length , nb_cells,nb_cells);

  Component& iterate = model.solver().access_component("iterate");
  iterate.configure_option("max_iter",1u);

  wizard.prepare_simulation();

  const Real r_L = 4.696;             const Real r_R = 1.408;
  const Real p_L = 404400;            const Real p_R = 101100;
  const Real u_L = 0.;                const Real u_R = 0.;
  const Real v_L = 0.;                const Real v_R = 0.;
  const Real g=1.4;

  RealVector4 left, right;
  left  << r_L , r_L*u_L , r_L*v_L,  p_L/(g-1) + 0.5*r_L*(u_L*u_L+v_R*v_R);
  right << r_R , r_R*u_R , r_R*v_R,  p_R/(g-1) + 0.5*r_R*(u_R*u_R+v_R*v_R);

  std::vector<std::string> function(4);
  for (Uint i=0; i<function.size(); ++i)
    function[i]="if(x<="+to_str(length/2.)+","+to_str(left[i])+","+to_str(right[i])+")";

  CFinfo << "\nInitializing solution with \n";
  boost_foreach(const std::string& f, function)
      CFinfo << "     " << f << CFendl;
  wizard.initialize_solution(function);

  MeshWriter& gmsh_writer = model.tools().create_component("gmsh","CF.Mesh.Gmsh.Writer").as_type<MeshWriter>();
  std::vector<URI> fields;
  fields.push_back(mesh.get_child("solution").uri());
  fields.push_back(mesh.get_child("residual").uri());
  fields.push_back(mesh.get_child("wave_speed").uri());
  fields.push_back(mesh.get_child("update_coeff").uri());
  fields.push_back(mesh.get_child("volume").uri());
  fields.push_back(mesh.get_child("jacobian_determinant").uri());
  gmsh_writer.configure_option("fields",fields);


  gmsh_writer.write_from_to(mesh,URI("2quads_euler_0.msh"));
  CFinfo << "Mesh \"2quads_euler_0.msh\" written" << CFendl;

  wizard.start_simulation(5.);

  gmsh_writer.write_from_to(mesh,URI("2quads_euler_1.msh"));

  CFinfo << "Mesh \"2quads_euler_1.msh\" written" << CFendl;

//  BOOST_CHECK_EQUAL (model.time().dt() , 2.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_adjoint )
{

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

