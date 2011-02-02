// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>
#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionURI.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"

#include "FVM/ForwardEuler.hpp"
#include "Solver/CDiscretization.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;

Common::ComponentBuilder < ForwardEuler, CIterativeSolver, LibFVM > ForwardEuler_Builder;

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler ( const std::string& name  ) : CIterativeSolver ( name )
{
  properties()["brief"] = std::string("Forward Euler Time Stepper");
  std::string description; 
  description += " 1) compute residual and update_coeff using discretization method\n";
  description += " 2) solution = update_coeff * residual\n";
  
  properties()["description"] = std::string("Forward Euler Time Stepper");

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &ForwardEuler::trigger_Domain,   this ) );

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &ForwardEuler::solve, this ) );
  // signal("solve").signature
  //     .insert<URI>("Domain", "Domain to load mesh into" )
  //     .insert_array<URI>( "Files" , "Files to read" );

  m_solution = create_static_component<CLink>("solution");
  m_residual = create_static_component<CLink>("residual");
  m_update_coeff = create_static_component<CLink>("update_coeff");
}

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::~ForwardEuler()
{
}

////////////////////////////////////////////////////////////////////////////////

void ForwardEuler::trigger_Domain()
{
  URI domain; property("Domain").put_value(domain);

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>(*look_component(domain));
  if (is_not_null(mesh))
  {
    CField2::Ptr solution = find_component_ptr_with_name<CField2>(*mesh,"solution");
    if ( is_null(solution) )
      solution = mesh->create_component<CField2>("solution");
    m_solution->link_to(solution);

    CField2::Ptr residual = find_component_ptr_with_name<CField2>(*mesh,"residual");
    if ( is_null(residual) )
      residual = mesh->create_component<CField2>("residual");
    m_residual->link_to(residual);

    CField2::Ptr update_coeff = find_component_ptr_with_name<CField2>(*mesh,"update_coeff");
    if ( is_null(update_coeff) )
      update_coeff = mesh->create_component<CField2>("update_coeff");
    m_update_coeff->link_to(update_coeff);

    discretization_method().configure_property( "Solution"    , solution->full_path() );
    discretization_method().configure_property( "Residual"    , residual->full_path() );
    discretization_method().configure_property( "UpdateCoeff" , update_coeff->full_path() );
        
  }
  else
  {
    throw ValueNotFound(FromHere(),"domain has no mesh ");
  }
}
//////////////////////////////////////////////////////////////////////////////

CDiscretization& ForwardEuler::discretization_method()
{
  return find_component<CDiscretization>(*this);
}

//////////////////////////////////////////////////////////////////////////////

void ForwardEuler::solve()
{
  if ( is_null(m_solution->follow()) )
    throw SetupError (FromHere(), "solution is not linked to solution field");

  if ( is_null(m_residual->follow()) )
    throw SetupError (FromHere(), "residual is not linked to solution field");

  if ( is_null(m_update_coeff->follow()) )
    throw SetupError (FromHere(), "update_coeff is not linked to solution field");
    
  CField2& solution     = *m_solution->follow()->as_type<CField2>();
  CField2& residual     = *m_residual->follow()->as_type<CField2>();
  CField2& update_coeff = *m_update_coeff->follow()->as_type<CField2>();

  //CFinfo << "Starting Iterative loop" << CFendl;
  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    residual.data() = 0.;
    update_coeff.data() = 0.;
    
    // compute RHS
    discretization_method().compute_rhs();

    //residual.data() *= update_coeff.data();
    solution.data() += residual.data();
    
    // compute norm
    Real rhs_L2=0;
    boost_foreach(CTable<Real>::ConstRow rhs , residual.data().array())
      rhs_L2 += rhs[0]*rhs[0];
    rhs_L2 = sqrt(rhs_L2) / residual.data().size();

    // output convergence info
    CFinfo << "Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
