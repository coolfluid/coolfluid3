// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>
#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"

#include "FVM/ForwardEuler.hpp"
#include "FVM/ComputeUpdateCoefficient.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/CDiscretization.hpp"

#include "Math/MathConsts.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;

Common::ComponentBuilder < ForwardEuler, CSolver, LibFVM > ForwardEuler_Builder;

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler ( const std::string& name  ) : CSolver ( name )
{
  properties()["brief"] = std::string("Forward Euler Time Stepper");
  std::string description =
    " U[n+1] = U[n] + dt/dx * R \n"
    " 1) delegate computation of the residual and wave_speed to the discretization method\n"
    " 2) compute the update coefficient = dt/dx = CFL/wave_speed"
    " 3) solution = update_coeff * residual\n";
  properties()["description"] = description;

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &ForwardEuler::trigger_Domain,   this ) );

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &ForwardEuler::solve, this ) );

  m_properties.add_option<OptionT<bool> >("OutputDiagnostics","Output information of convergence",false)->mark_basic();

  m_solution = create_static_component<CLink>("solution");
  m_residual = create_static_component<CLink>("residual");
  m_wave_speed = create_static_component<CLink>("wave_speed");
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
  if (is_null(mesh))
    throw SetupError(FromHere(),"Domain has no mesh");

  discretization_method().configure_property("Mesh",mesh->full_path());

  CField2& solution = find_component_with_tag<CField2>(*mesh,"solution");
  m_solution->link_to(solution.self());    

  Component::Ptr residual_ptr = find_component_ptr_with_tag(*mesh,"residual");
  if ( is_null(residual_ptr) )
  {
    residual_ptr = mesh->create_field2("residual",solution).self();
    residual_ptr->add_tag("residual");
  }
  m_residual->link_to(residual_ptr);

  Component::Ptr wave_speed_ptr = find_component_ptr_with_tag(*mesh,"wave_speed");
  if ( is_null(wave_speed_ptr) )
  {
    wave_speed_ptr = mesh->create_scalar_field("wave_speed",solution).self();
    wave_speed_ptr->add_tag("wave_speed");
  }
  m_wave_speed->link_to(wave_speed_ptr);

  Component::Ptr update_coeff_ptr = find_component_ptr_with_tag(*mesh,"update_coeff");
  if ( is_null(update_coeff_ptr) )
  {
    update_coeff_ptr = mesh->create_scalar_field("update_coeff",solution).self();
    update_coeff_ptr->add_tag("update_coeff");
  }
  m_update_coeff->link_to(update_coeff_ptr);

  configure_option_recursively("solution",solution.full_path());
  configure_option_recursively("wave_speed",wave_speed_ptr->full_path());
  configure_option_recursively("residual",residual_ptr->full_path());
  configure_option_recursively("update_coeff",update_coeff_ptr->full_path());
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
    
  CField2& solution     = *m_solution->follow()->as_ptr<CField2>();
  CField2& residual     = *m_residual->follow()->as_ptr<CField2>();
  CField2& wave_speed    = *m_wave_speed->follow()->as_ptr<CField2>();
  CField2& update_coeff = *m_update_coeff->follow()->as_ptr<CField2>();

  //CFinfo << "Starting Iterative loop" << CFendl;
  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    // initialize loop
    residual.data() = 0.;
    wave_speed.data() = Math::MathConsts::eps();
    
    // compute residual = flux_in - flux_out
    discretization_method().get_child<CAction>("apply_boundary_conditions")->execute();
    discretization_method().get_child<CAction>("compute_rhs")->execute();

    // Compute the update coefficient = dt/dx = CFL/wave_speed
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
