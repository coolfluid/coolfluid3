// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllT.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

#include "RDM/ResidualDistribution.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {

Common::ComponentBuilder < ResidualDistribution, CDiscretization, LibRDM > ResidualDistribution_Builder;

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::ResidualDistribution ( const std::string& name  ) :
  CDiscretization ( name )
{
  // properties

  properties()["brief"] = std::string("Residual Distribution Method");

  std::string desc =
        "Discretize the PDE's using the Cell Centered Finite Volume Method\n"
        "This method ... (explain)";
  properties()["description"] = desc ;

  m_properties["Mesh"].as_option().attach_trigger ( boost::bind ( & ResidualDistribution::config_mesh, this ) );

  // signals

  regist_signal ("signal_create_boundary_term" ,
                 "creates a boundary condition term",
                 "Create Boundary Condition" )
      ->connect ( boost::bind ( &ResidualDistribution::signal_create_boundary_term, this, _1 ) );

  signal("signal_create_boundary_term").signature
      ->connect( boost::bind( &ResidualDistribution::signature_signal_create_boundary_term, this, _1));

  regist_signal ("create_domain_term",
                 "creates a domain volume term",
                 "Create Domain Term" )
      ->connect ( boost::bind ( &ResidualDistribution::signal_create_domain_term, this, _1 ) );

  signal("create_domain_term").signature
      ->connect( boost::bind( &ResidualDistribution::signature_create_domain_term, this, _1));

  // setup of the static components

  // create apply boundary conditions action
  m_compute_boundary_face_terms = create_static_component<CAction>("compute_boundary_faces");
  m_compute_boundary_face_terms->mark_basic();

  // create compute rhs action
  m_compute_volume_cell_terms = create_static_component<CAction>("compute_volume_cells");
  m_compute_volume_cell_terms->mark_basic();
}


////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution() {}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::config_mesh()
{
  CMesh& mesh = *( m_mesh.lock() );

  // check if a solution field exist, and create if does not

  std::string solution_tag("solution");
  CField2::Ptr solution = find_component_ptr_with_tag<CField2>( mesh, solution_tag );
  if ( is_null(solution) )
  {
    CFinfo << " +++ creating solution field " << CFendl;
    solution = mesh.create_field2("solution","PointBased","u[1]").as_type<CField2>();
    solution->add_tag(solution_tag);
  }

  //    CFinfo << " - solution field : " << m_solution.lock()->full_path().string() << CFendl;

  /// @todo should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::signal_create_boundary_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  std::string name = options.get_option<std::string>("Name");
  std::string type = options.get_option<std::string>("Type");

  std::vector<URI> regions = options.get_array<URI>("Regions");

// this will be for neunamm bcs
//  CAction& face_loop =
//      m_compute_boundary_face_terms->create_action("CF.Solver.Actions.CForAllFaces", name);

  CAction::Ptr face_loop = create_component_abstract_type<CLoop>("CF.Solver.Actions.CForAllNodes2",name);
  m_compute_boundary_face_terms->add_component(face_loop);

  face_loop->configure_property("Regions" , regions);
  face_loop->mark_basic();

  CAction& face_action = face_loop->create_action( type , "action" );
  face_action.mark_basic();

  face_action.configure_property("Mesh", m_mesh.lock()->full_path());
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::signature_signal_create_boundary_term( Signal::arg_t& node )
{
  const bool basic = true;

  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  // name
  options.set_option<std::string>("Name", std::string(), "Name for created boundary term" );

  // type
  std::vector< std::string > restricted;
  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  XmlNode type_node = options.set_option<std::string>("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.set_array<URI>("Regions", dummy , "Regions where to apply the boundary condition", " ; " );
}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::signal_create_domain_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  std::string name = options.get_option<std::string>("Name");
  std::string type = options.get_option<std::string>("Type");

  std::vector<URI> regions = options.get_array<URI>("Regions");

  CAction& cell_loop = m_compute_volume_cell_terms->create_action(type, name);

  cell_loop.configure_property("Regions" , regions);
  cell_loop.mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::signature_create_domain_term( Signal::arg_t& node )
{
  const bool basic = true;

  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  // name
  options.set_option<std::string>("Name", std::string(), "Name for created volume term" );

  // type
  std::vector< std::string > restricted;
  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  XmlNode type_node = options.set_option<std::string>("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.set_array<URI>("Regions", dummy , "Regions where to apply the boundary condition", " ; " );
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::compute_rhs()
{
  //  CFinfo << " --  computing boundary face terms" << CFendl;
  m_compute_boundary_face_terms->execute();

  //  CFinfo << " --  computing volume cell terms" << CFendl;
  m_compute_volume_cell_terms->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
