// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/OptionArray.hpp"
#include "common/Log.hpp"

#include "common/PE/Comm.hpp"


#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Time.hpp"
#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "physics/Variables.hpp"

#include "sdm/Tags.hpp"

#include "sdm/lineuler/ComputeCharacteristicVariables.hpp"

#include "Physics/LinEuler/LinEuler2D.hpp"

#include "math/Consts.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;

namespace cf3 {
namespace sdm {
namespace lineuler {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeCharacteristicVariables, common::Action, LibLinEuler > ComputeCharacteristicVariables_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeCharacteristicVariables::ComputeCharacteristicVariables ( const std::string& name ) :
  solver::Action(name)
{
  mark_basic();

  std::vector<Real> normal(2);
  normal[XX]=1.;
  normal[YY]=0.;
  options().add_option("normal",normal).description("characteristic normal");
}

////////////////////////////////////////////////////////////////////////////////

void ComputeCharacteristicVariables::execute()
{
  Field& solution = *follow_link(solver().field_manager().get_child(sdm::Tags::solution()))->handle<Field>();
  Field& residual = *follow_link(solver().field_manager().get_child(sdm::Tags::residual()))->handle<Field>();
  Field& convection = *solution.dict().get_child("convection")->handle<Field>();

  physics::Variables& sol_vars = *physical_model().get_child(sdm::Tags::solution_vars())->handle<physics::Variables>();
  RealVector dummy_coords(physical_model().ndim());
  RealMatrix dummy_grads(physical_model().neqs(),physical_model().ndim());
  RealVector sol_vec(physical_model().neqs());

  RealVector n(physical_model().ndim());
  std::vector<Real> normal = options().option("normal").value< std::vector<Real> >();
  n[XX]=normal[XX];
  n[YY]=normal[YY];

  if (is_null(m_characteristics))
  {
    if (Handle<Component> found_characteristics = solver().field_manager().get_child("char"))
    {
      m_characteristics = found_characteristics->handle<Field>();
    }
    else
    {
      m_characteristics = mesh().get_child("solution_space")->handle<Dictionary>()->create_field("char","S,Vort,Aplus,Amin,A,omega").handle<Field>();
      solver().field_manager().create_component<Link>("char")->link_to(*m_characteristics);
    }
  }
  Field& char_field = *m_characteristics;


  if (is_null(m_char_resid))
  {
    if (Handle<Component> found_characteristics = solver().field_manager().get_child("char_resid"))
    {
      m_char_resid = found_characteristics->handle<Field>();
    }
    else
    {
      m_char_resid = mesh().get_child("solution_space")->handle<Dictionary>()->create_field("char_resid","rhs_S,rhs_Vort,rhs_Aplus,rhs_Amin,rhs_A,rhs_omega").handle<Field>();
      solver().field_manager().create_component<Link>("char_resid")->link_to(*m_char_resid);
    }
  }
  Field& char_resid_field = *m_char_resid;


  if (is_null(m_char_convection))
  {
    if (Handle<Component> found_characteristics = solver().field_manager().get_child("char_convection"))
    {
      m_char_convection = found_characteristics->handle<Field>();
    }
    else
    {
      m_char_convection = mesh().get_child("solution_space")->handle<Dictionary>()->create_field("char_convection","conv_S,conv_Vort,conv_Aplus,conv_Amin,conv_A,conv_omega").handle<Field>();
      solver().field_manager().create_component<Link>("char_convection")->link_to(*m_char_convection);
    }
  }
  Field& char_convection_field = *m_char_convection;


  std::auto_ptr<physics::Properties> phys_props = physical_model().create_properties();
  physics::LinEuler::LinEuler2D::Properties& p = static_cast<physics::LinEuler::LinEuler2D::Properties&>( *phys_props );

//  CFinfo << "n = " << n.transpose() << CFendl;
  boost_foreach(const Handle<Entities>& elements, char_field.entities_range()  )
  {
    const Space& space = char_field.space(*elements);
    for (Uint elem=0; elem<elements->size(); ++elem)
    {
      boost_foreach(Uint idx, space.connectivity()[elem])
      {
        for (Uint v=0; v<physical_model().neqs(); ++v)
          sol_vec[v] = solution[idx][v];
        sol_vars.compute_properties(dummy_coords,sol_vec,dummy_grads,p);

        // S, Omega, Aplus, Amin, A, omega
        char_field[idx][0] = p.rho - p.p*p.inv_c*p.inv_c;
        char_field[idx][1] =  n[YY]*p.rho0u - n[XX]*p.rho0v;
        char_field[idx][2] =  n[XX]*p.rho0u + n[YY]*p.rho0v + p.p*p.inv_c;
        char_field[idx][3] = -n[XX]*p.rho0u - n[YY]*p.rho0v + p.p*p.inv_c;
        char_field[idx][4] = 2. * p.inv_c * p.p;
        char_field[idx][5] = 2. * (  p.rho0 * ( p.u*n[XX] + p.v*n[YY] )  );

////        CFinfo << "A+ = " << char_field[idx][2] << "   A- = " << char_field[idx][3] << CFendl;
        for (Uint v=0; v<physical_model().neqs(); ++v)
          sol_vec[v] = residual[idx][v];
        sol_vars.compute_properties(dummy_coords,sol_vec,dummy_grads,p);

        // S, Omega, Aplus, Amin, A, omega
        char_resid_field[idx][0] = p.rho - p.p*p.inv_c*p.inv_c;
        char_resid_field[idx][1] =  n[YY]*p.rho0u - n[XX]*p.rho0v;
        char_resid_field[idx][2] =  n[XX]*p.rho0u + n[YY]*p.rho0v + p.p*p.inv_c;
        char_resid_field[idx][3] = -n[XX]*p.rho0u - n[YY]*p.rho0v + p.p*p.inv_c;
        char_resid_field[idx][4] = 2. * p.inv_c * p.p;
        char_resid_field[idx][5] = 2. * (  p.rho0 * ( p.u*n[XX] + p.v*n[YY] )  );


        for (Uint v=0; v<physical_model().neqs(); ++v)
          sol_vec[v] = -convection[idx][v];
        sol_vars.compute_properties(dummy_coords,sol_vec,dummy_grads,p);

        // S, Omega, Aplus, Amin, A, omega
        char_convection_field[idx][0] = p.rho - p.p*p.inv_c*p.inv_c;
        char_convection_field[idx][1] =  n[YY]*p.rho0u - n[XX]*p.rho0v;
        char_convection_field[idx][2] =  n[XX]*p.rho0u + n[YY]*p.rho0v + p.p*p.inv_c;
        char_convection_field[idx][3] = -n[XX]*p.rho0u - n[YY]*p.rho0v + p.p*p.inv_c;
        char_convection_field[idx][4] = 2. * p.inv_c * p.p;
        char_convection_field[idx][5] = 2. * (  p.rho0 * ( p.u*n[XX] + p.v*n[YY] )  );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

