// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionArray.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/StringConversion.hpp"

#include "Math/MathChecks.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"

#include "RDM/RKRD.hpp"
#include "RDM/DomainTerm.hpp"
#include "RDM/BoundaryTerm.hpp"
#include "RDM/Cleanup.hpp"

namespace CF {
namespace RDM {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Math::MathChecks;

Common::ComponentBuilder < RKRD, CSolver, LibRDM > RKRD_Builder;

////////////////////////////////////////////////////////////////////////////////

RKRD::RKRD ( const std::string& name  ) :
  CSolver ( name ),
  m_cfl(1.0),
  m_nb_iter(0)
{
  // options

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &RKRD::config_domain,   this ) );

  m_properties.add_option<OptionT <Uint> >("MaxIter",
                                           "Maximum number of iterations",
                                            m_nb_iter)
      ->mark_basic()
      ->link_to( &m_nb_iter );
  
  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_cfl)
      ->mark_basic()
      ->link_to(&m_cfl)
      ->add_tag("cfl");

  m_properties.add_option( OptionComponent<CField2>::create("Solution","Solution field",&m_solution) )
      ->add_tag("solution");

  m_properties.add_option( OptionComponent<CField2>::create("Residual","Residual field",&m_residual) )
      ->add_tag("residual");

  m_properties.add_option( OptionComponent<CField2>::create("Update Coeffs","Update coefficients field",&m_wave_speed) )
      ->add_tag("wave_speed");

  m_properties.add_option(OptionComponent<CMesh>::create("Mesh","Mesh the Discretization Method will be applied to",&m_mesh))
    ->attach_trigger ( boost::bind ( &RKRD::config_mesh,   this ) );
  properties()["Mesh"].as_option().add_tag("mesh");

  // properties

  properties()["brief"] = std::string("Iterative RDM component");
  properties()["description"] = std::string("Forward Euler Time Stepper");

  // signals

  regist_signal ("create_boundary_term" ,
                 "creates a boundary condition term",
                 "Create Boundary Condition" )
      ->connect ( boost::bind ( &RKRD::signal_create_boundary_term, this, _1 ) );

  signal("create_boundary_term").signature
      ->connect( boost::bind( &RKRD::signature_signal_create_boundary_term, this, _1));

  regist_signal ("create_domain_term",
                 "creates a domain volume term",
                 "Create Domain Term" )
      ->connect ( boost::bind ( &RKRD::signal_create_domain_term, this, _1 ) );

  signal("create_domain_term").signature
      ->connect( boost::bind( &RKRD::signature_create_domain_term, this, _1));

  // setup of the static components

  // create apply boundary conditions action
  m_compute_boundary_face_terms = create_static_component<CAction>("compute_boundary_faces");
  m_compute_boundary_face_terms->mark_basic();

  // create compute rhs action
  m_compute_volume_cell_terms = create_static_component<CAction>("compute_volume_cells");
  m_compute_volume_cell_terms->mark_basic();

  // create apply boundary conditions action
  m_cleanup = create_static_component<Cleanup>("cleanup");

}

////////////////////////////////////////////////////////////////////////////////

RKRD::~RKRD() {}

////////////////////////////////////////////////////////////////////////////////

void RKRD::config_domain()
{
  CDomain::Ptr domain = access_component_ptr( property("Domain").value<URI>() )->as_ptr<CDomain>();
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not point to Domain");

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>( *domain );

//  CFinfo << "mesh " << mesh->full_path().string() << CFendl;
  if (is_not_null(mesh))
  {
    m_mesh = mesh;
    config_mesh();
  }
}

////////////////////////////////////////////////////////////////////////////////

void RKRD::config_mesh()
{
  if( is_null(m_mesh.lock()) )
      return;

  CMesh& mesh = *(m_mesh.lock());

  // configure solution
  std::string solution_tag("solution");
  m_solution = find_component_ptr_with_tag<CField2>( mesh, solution_tag );
  if ( is_null( m_solution.lock() ) )
  {
//    CFinfo << " +++ creating solution field " << CFendl;
    m_solution = mesh.create_field2("solution","PointBased","u[1]").as_ptr<CField2>();
    m_solution.lock()->add_tag(solution_tag);
  }

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual
  std::string residual_tag("residual");
  m_residual = find_component_ptr_with_tag<CField2>( mesh, residual_tag);
  if ( is_null( m_residual.lock() ) )
  {
//    CFinfo << " +++ creating residual field " << CFendl;
    m_residual = mesh.create_field2("residual",*m_solution.lock()).as_ptr<CField2>();
    m_residual.lock()->add_tag(residual_tag);
  }

  // configure wave_speed
  std::string wave_speed_tag("wave_speed");
  m_wave_speed = find_component_ptr_with_tag<CField2>( mesh, wave_speed_tag);
  if ( is_null(m_wave_speed.lock()) )
  {
//    CFinfo << " +++ creating wave_speed field " << CFendl;
    m_wave_speed = mesh.create_scalar_field("wave_speed",*m_solution.lock()).as_ptr<CField2>();
    m_wave_speed.lock()->add_tag(wave_speed_tag);
  }

  std::vector<URI> cleanup_fields;
  cleanup_fields.push_back( m_residual.lock()->full_path() );
  cleanup_fields.push_back( m_wave_speed.lock()->full_path() );
  m_cleanup->configure_property("Fields", cleanup_fields);

}

//////////////////////////////////////////////////////////////////////////////

void RKRD::solve()
{
  // ensure domain is sane
  CDomain::Ptr domain = access_component_ptr( property("Domain").value<URI>() )->as_ptr<CDomain>();
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not poitn to Domain");

  CTable<Real>& solution     = m_solution.lock()->data();
  CTable<Real>& residual     = m_residual.lock()->data();
  CTable<Real>& wave_speed = m_wave_speed.lock()->data();

  // initialize to zero condition

  /// @todo should be moved out of here
  const Uint size = residual.size();
  for (Uint i=0; i<size; ++i)
    solution[i][0]=0.;

  CFinfo << " - starting iterative loop" << CFendl;

  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    /// @todo move this into an action

    // cleanup needed fields (typically residual and wave_speed)
    m_cleanup->execute();

    // compute RHS

    //  CFinfo << " --  computing boundary face terms" << CFendl;
    m_compute_boundary_face_terms->execute();

    //  CFinfo << " --  computing volume cell terms" << CFendl;
    m_compute_volume_cell_terms->execute();

    // explicit update
    const Uint nbdofs = solution.size();
    for (Uint i=0; i< nbdofs; ++i)
      solution[i][0] += - ( m_cfl / wave_speed[i][0] ) * residual[i][0];

    //  computing the norm
    Real rhs_L2 = 0.;
    for (Uint i=0; i< nbdofs; ++i)
      rhs_L2 += residual[i][0] * residual[i][0];
    rhs_L2 = sqrt(rhs_L2)/nbdofs;

    // output convergence info
    CFinfo << "Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
    if ( is_nan(rhs_L2) || is_inf(rhs_L2) )
      throw FailedToConverge(FromHere(),"Solution diverged after "+to_str(iter)+" iterations");
  }
}

//////////////////////////////////////////////////////////////////////////////

void RKRD::signal_create_boundary_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  std::string name = options.get_option<std::string>("Name");
  std::string type = options.get_option<std::string>("Type");

  std::vector<URI> regions = options.get_array<URI>("Regions");

  RDM::BoundaryTerm::Ptr bterm = create_component_abstract_type<RDM::BoundaryTerm>(type,name);
  m_compute_boundary_face_terms->add_component(bterm);

  bterm->configure_property("Regions" , regions);
  bterm->configure_property("Mesh", m_mesh.lock()->full_path());
}

////////////////////////////////////////////////////////////////////////////////

void RKRD::signature_signal_create_boundary_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  // name
  options.set_option<std::string>("Name", std::string(), "Name for created boundary term" );

  // type
  std::vector< std::string > restricted;
//  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  XmlNode type_node = options.set_option<std::string>("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.set_array<URI>("Regions", dummy , "Regions where to apply the boundary condition", " ; " );
}

//////////////////////////////////////////////////////////////////////////////

void RKRD::signal_create_domain_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  std::string name = options.get_option<std::string>("Name");
  std::string type = options.get_option<std::string>("Type");

  RDM::DomainTerm::Ptr dterm = create_component_abstract_type<RDM::DomainTerm>(type,name);
  m_compute_volume_cell_terms->add_component(dterm);

  std::vector<URI> regions = options.get_array<URI>("Regions");

  dterm->configure_property("Regions" , regions);
}

////////////////////////////////////////////////////////////////////////////////

void RKRD::signature_create_domain_term( Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  // name
  options.set_option<std::string>("Name", std::string(), "Name for created volume term" );

  // type
//  std::vector< std::string > restricted;
//  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
//  XmlNode type_node = options.set_option<std::string>("Type", std::string("CF.RDM.BcDirichlet"), "Type for created boundary");
//  Map(type_node).set_array( Protocol::Tags::key_restricted_values(), restricted, " ; " );

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.set_array<URI>("Regions", dummy , "Regions where to apply the domain term", " ; " );
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
