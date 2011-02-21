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

  // options

  m_properties.add_option( OptionComponent<CField2>::create("Solution","Solution field",&m_solution) )
    ->mark_basic()
    ->add_tag("solution");

  m_properties.add_option( OptionComponent<CField2>::create("Residual","Residual field",&m_residual) )
    ->mark_basic()
    ->add_tag("residual");

  m_properties.add_option( OptionComponent<CField2>::create("Update Coeffs","Update coefficients field",&m_update_coeff) )
    ->mark_basic()
    ->add_tag("update_coeff");

  // signals

  regist_signal ( "create_boundary_term" , "creates a boundary condition term", "Create Boundary Condition" )->connect ( boost::bind ( &ResidualDistribution::create_boundary_term, this, _1 ) );
  regist_signal ( "create_domain_term" , "creates a domain volume term", "Create Domain Term" )->connect ( boost::bind ( &ResidualDistribution::create_domain_term, this, _1 ) );
    
  // setup of the static components

  // create apply boundary conditions action
  m_compute_boundary_face_terms = create_static_component<CAction>("compute_boundary_faces");
  m_compute_boundary_face_terms->mark_basic();

  // create compute rhs action
  m_compute_volume_cell_terms = create_static_component<CAction>("compute_volume_cells");
  m_compute_volume_cell_terms->mark_basic();

  m_solution_field =     create_static_component<CLink>( "solution" );
  m_residual_field =     create_static_component<CLink>( "residual" );
  m_update_coeff_field = create_static_component<CLink>( "update_coeff" );
}


////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution() {}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::config_mesh()
{
  CMesh& mesh = *m_mesh.lock();

  CField2::Ptr solution     = find_component_ptr_with_name<CField2>(mesh,"solution");
  CField2::Ptr residual     = find_component_ptr_with_name<CField2>(mesh,"residual");
  CField2::Ptr update_coeff = find_component_ptr_with_name<CField2>(mesh,"update_coeff");

  m_solution_field->link_to(solution);
  m_residual_field->link_to(residual);
  m_update_coeff_field->link_to(update_coeff);
}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::create_boundary_term( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");
  std::string type = p.get_option<std::string>("Type");

  std::vector<URI> regions = p.get_array<URI>("Regions");

// this will be for neunamm bcs
//  CAction& face_loop =
//      m_compute_boundary_face_terms->create_action("CF.Solver.Actions.CForAllFaces", name);

  CAction::Ptr face_loop = create_component_abstract_type<CLoop>("CF.Solver.Actions.CForAllNodes2",name);
  m_compute_boundary_face_terms->add_component(face_loop);

  face_loop->configure_property("Regions" , regions);
  face_loop->mark_basic();

  CAction& face_action = face_loop->create_action( type , "action" );
  face_action.mark_basic();

  face_action.configure_property("Solution", m_solution_field->follow()->full_path());
}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::create_domain_term( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");
  std::string type = p.get_option<std::string>("Type");

  std::vector<URI> regions = p.get_array<URI>("Regions");

  CAction& cell_loop = m_compute_volume_cell_terms->create_action(type, name);

  cell_loop.configure_property("Regions" , regions);
  cell_loop.mark_basic();
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
