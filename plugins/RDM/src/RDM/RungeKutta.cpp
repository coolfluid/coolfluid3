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
#include "Common/String/Conversion.hpp"

#include "Math/MathChecks.hpp"

#include "RDM/RungeKutta.hpp"
#include "Solver/CDiscretization.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

namespace CF {
namespace RDM {

using namespace Common;
using namespace Common::String;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Math::MathChecks;

Common::ComponentBuilder < RungeKutta, CIterativeSolver, LibRDM > RungeKutta_Builder;

////////////////////////////////////////////////////////////////////////////////

RungeKutta::RungeKutta ( const std::string& name  ) :
  CIterativeSolver ( name ),
  m_cfl(1.0)
{
  // options

  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_cfl)
      ->mark_basic()
      ->link_to(&m_cfl)
      ->add_tag("cfl");

  m_properties.add_option( OptionComponent<CField2>::create("Solution","Solution field",&m_solution) )
      ->add_tag("solution");

  m_properties.add_option( OptionComponent<CField2>::create("Residual","Residual field",&m_residual) )
      ->add_tag("residual");

  m_properties.add_option( OptionComponent<CField2>::create("Update Coeffs","Update coefficients field",&m_update_coeff) )
      ->add_tag("update_coeff");

  // properties

  properties()["brief"] = std::string("Iterative RDM component");
  properties()["description"] = std::string("Forward Euler Time Stepper");

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &RungeKutta::config_domain,   this ) );

  // signals

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &RungeKutta::solve, this ) );  

}

////////////////////////////////////////////////////////////////////////////////

RungeKutta::~RungeKutta()
{
}

////////////////////////////////////////////////////////////////////////////////

void RungeKutta::config_domain()
{
  CDomain::Ptr domain = look_component<CDomain>( property("Domain").value<URI>() );
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not point to Domain");

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>( *domain );
  if (is_not_null(mesh))
  {
    // configure DM mesh first to ensure that solution is there
    discretization_method().configure_property( "Mesh" , mesh->full_path() );

    CField2::Ptr solution = find_component_ptr_with_tag<CField2>(*mesh, "solution");
    cf_assert( is_not_null(solution) );
    m_solution = solution;

    // configure residual
    std::string residual_tag("residual");
    m_residual = find_component_ptr_with_tag<CField2>(*mesh, residual_tag);
    if ( is_null( m_residual.lock() ) )
    {
      CFinfo << " +++ creating residual field " << CFendl;
      m_residual = mesh->create_field2("residual",*solution).as_type<CField2>();
      m_residual.lock()->add_tag(residual_tag);
    }

    // configure update_coeff
    std::string update_coeff_tag("update_coeff");
    m_update_coeff = find_component_ptr_with_tag<CField2>(*mesh, update_coeff_tag);
    if ( is_null(m_update_coeff.lock()) )
    {
      CFinfo << " +++ creating update_coeff field " << CFendl;
      m_update_coeff = mesh->create_scalar_field("update_coeff",*solution).as_type<CField2>();
      m_update_coeff.lock()->add_tag(update_coeff_tag);
    }

  }
}

//////////////////////////////////////////////////////////////////////////////

CDiscretization& RungeKutta::discretization_method()
{
  return find_component<CDiscretization>(*this);
}

//////////////////////////////////////////////////////////////////////////////

void RungeKutta::solve()
{
  // ensure domain is sane
  CDomain::Ptr domain = look_component<CDomain>( property("Domain").value<URI>() );
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not poitn to Domain");

  CTable<Real>& solution     = m_solution.lock()->data();
  CTable<Real>& residual     = m_residual.lock()->data();
  CTable<Real>& update_coeff = m_update_coeff.lock()->data();

//  CFinfo << "DATA TABLE SIZES:" << CFendl;
//  CFinfo << "solution: " << solution.size() << " x " << solution.row_size() << CFendl;
//  CFinfo << "residual: " << residual.size() << " x " << residual.row_size() << CFendl;
//  CFinfo << "update_coeff: " << update_coeff.size() << " x " << update_coeff.row_size() << CFendl;

//  CFinfo << " - initializing solution" << CFendl;

  // initialize to zero condition

  /// @todo should be moved out of here
  Uint size = residual.size();
  for (Uint i=0; i<size; ++i)
    solution[i][0]=0;

  CFinfo << " - starting iterative loop" << CFendl;

  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    /// @todo move this into an action

    // set update coefficient and residual to zero

    Uint size = residual.size();
    for (Uint i=0; i<size; ++i)
      residual[i][0]=0;

    size = update_coeff.size();
    for (Uint i=0; i<size; ++i)
      update_coeff[i][0]=0;


    // compute RHS
    discretization_method().compute_rhs();

    // explicit update
    const Uint nbdofs = solution.size();
    for (Uint i=0; i< nbdofs; ++i)
      solution[i][0] += - ( m_cfl / update_coeff[i][0] ) * residual[i][0];

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

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
