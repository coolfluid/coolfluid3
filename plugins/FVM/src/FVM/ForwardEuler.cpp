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
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"

#include "FVM/ForwardEuler.hpp"
#include "FVM/ComputeUpdateCoefficient.hpp"


#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllNodes.hpp"
#include "Solver/CDiscretization.hpp"

#include "Math/MathConsts.hpp"

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
  std::string description =
    " U[n+1] = U[n] + dt/dx * R \n"
    " 1) delegate computation of the residual and advection to the discretization method\n"
    " 2) compute the update coefficient = dt/dx = CFL/advection"
    " 3) solution = update_coeff * residual\n";
  properties()["description"] = description;

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &ForwardEuler::trigger_Domain,   this ) );

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &ForwardEuler::solve, this ) );

  m_properties.add_option<OptionT<bool> >("OutputDiagnostics","Output information of convergence",false)->mark_basic();
  m_properties.add_option<OptionURI>("Solution","Solution to march in time",URI())
    ->attach_trigger ( boost::bind ( &ForwardEuler::trigger_solution,   this ) );

  m_solution = create_static_component<CLink>("solution");
  m_residual = create_static_component<CLink>("residual");
  m_advection = create_static_component<CLink>("advection");
  m_update_coeff = create_static_component<CLink>("update_coeff");
  
  m_compute_update_coefficient = create_static_component<ComputeUpdateCoefficient>("compute_update_coeff");
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
    CFinfo << "domain has mesh" << CFendl;
    configure_option_recursively("mesh",mesh->full_path());
  }
  else
  {
    throw ValueNotFound(FromHere(),"domain has no mesh ");
  }
}

////////////////////////////////////////////////////////////////////////////////

void ForwardEuler::trigger_solution()
{
  URI uri; property("Solution").put_value(uri);
  m_solution->link_to(look_component(uri));
  
  CField2& solution = *m_solution->follow()->as_type<CField2>();
  CMesh::Ptr mesh = solution.get_parent()->as_type<CMesh>();
  if (is_null(mesh)) throw SetupError (FromHere(), "Solution must be located inside a CMesh");

  CField2& residual = mesh->create_field2("residual",solution);
  m_residual->link_to(residual.self());
  configure_option_recursively("residual",residual.full_path());
  
  CField2& advection = mesh->create_scalar_field("advection",solution);
  m_advection->link_to(advection.self());
  configure_option_recursively("advection",advection.full_path());
  
  CField2& update_coeff = mesh->create_scalar_field("update_coeff",solution);
  m_update_coeff->link_to(update_coeff.self());
  configure_option_recursively("update_coeff",update_coeff.full_path());
  
  configure_option_recursively("solution",solution.full_path());
  
}

//////////////////////////////////////////////////////////////////////////////

CDiscretization& ForwardEuler::discretization_method()
{
  return find_component<CDiscretization>(*this);
}

//////////////////////////////////////////////////////////////////////////////

void ForwardEuler::solve()
{
  if ( is_null(m_solution->follow()) )  throw SetupError (FromHere(), "solution is not linked to solution field");
    
  CField2& solution     = *m_solution->follow()->as_type<CField2>();
  CField2& residual     = *m_residual->follow()->as_type<CField2>();
  CField2& advection    = *m_advection->follow()->as_type<CField2>();
  CField2& update_coeff = *m_update_coeff->follow()->as_type<CField2>();

  //CFinfo << "Starting Iterative loop" << CFendl;
  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    // initialize loop
    residual.data() = 0.;
    advection.data() = Math::MathConsts::eps();
    
    // compute residual = flux_in - flux_out
    discretization_method().get_child<CAction>("apply_boundary_conditions")->execute();
    discretization_method().get_child<CAction>("compute_rhs")->execute();

    // Compute the update coefficient = dt/dx = CFL/advection
    m_compute_update_coefficient->execute();

    residual.data() *= update_coeff.data();
    
    // update solution = old_solution  + dt/dx * (flux_in - flux_out)
    solution.data() += residual.data();
    
    discretization_method().get_child<CAction>("apply_boundary_conditions")->execute();
    
    if (property("OutputDiagnostics").value<bool>())
    {
      // compute norm
      Real rhs_L2=0;
      boost_foreach(CTable<Real>::ConstRow rhs , residual.data().array())
        rhs_L2 += rhs[0]*rhs[0];
      rhs_L2 = sqrt(rhs_L2) / residual.data().size();

      // output convergence info
      CFinfo << "ForwardEuler Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
