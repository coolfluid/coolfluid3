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
  options().add("normal",normal).description("characteristic normal").mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeCharacteristicVariables::execute()
{
  Field& solution = *follow_link(solver().field_manager().get_child(sdm::Tags::solution()))->handle<Field>();
  Field& residual = *follow_link(solver().field_manager().get_child(sdm::Tags::residual()))->handle<Field>();

  physics::Variables& sol_vars = *physical_model().get_child(sdm::Tags::solution_vars())->handle<physics::Variables>();
  RealVector dummy_coords(physical_model().ndim());
  RealMatrix dummy_grads(physical_model().neqs(),physical_model().ndim());
  RealVector sol_vec(physical_model().neqs());

  RealVector n(physical_model().ndim());
  std::vector<Real> normal = options().value< std::vector<Real> >("normal");
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


  if (is_null(m_dchardn))
  {
    if (Handle<Component> found_characteristics = solver().field_manager().get_child("dchardn"))
    {
      m_dchardn = found_characteristics->handle<Field>();
    }
    else
    {
      m_dchardn = mesh().get_child("solution_space")->handle<Dictionary>()->create_field("dchardn","dSdn,dOmegadn,dAplusdn,dAmindn,dAdn,domegadn").handle<Field>();
      solver().field_manager().create_component<Link>("dchardn")->link_to(*m_dchardn);
    }
  }
  Field& dchardn = *m_dchardn;

  if (is_null(m_dchards))
  {
    if (Handle<Component> found_characteristics = solver().field_manager().get_child("dchards"))
    {
      m_dchards = found_characteristics->handle<Field>();
    }
    else
    {
      m_dchards = mesh().get_child("solution_space")->handle<Dictionary>()->create_field("dchards","dSds,dOmegads,dAplusds,dAminds,dAds,domegads").handle<Field>();
      solver().field_manager().create_component<Link>("dchards")->link_to(*m_dchards);
    }
  }
  Field& dchards = *m_dchards;

  std::auto_ptr<physics::Properties> phys_props = physical_model().create_properties();
  physics::LinEuler::LinEuler2D::Properties& p = static_cast<physics::LinEuler::LinEuler2D::Properties&>( *phys_props );

  boost_foreach(const Handle<Entities>& elements, char_field.entities_range()  )
  {
    const Space& space = char_field.space(*elements);
    RealMatrix grad_sf(space.shape_function().dimensionality(),space.shape_function().nb_nodes());

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
        char_field[idx][4] = p.inv_c * p.p;
        char_field[idx][5] = (  p.rho0 * ( p.u*n[XX] + p.v*n[YY] )  );

////        CFinfo << "A+ = " << char_field[idx][2] << "   A- = " << char_field[idx][3] << CFendl;
        for (Uint v=0; v<physical_model().neqs(); ++v)
          sol_vec[v] = residual[idx][v];
        sol_vars.compute_properties(dummy_coords,sol_vec,dummy_grads,p);

        // S, Omega, Aplus, Amin, A, omega
        char_resid_field[idx][0] = p.rho - p.p*p.inv_c*p.inv_c;
        char_resid_field[idx][1] =  n[YY]*p.rho0u - n[XX]*p.rho0v;
        char_resid_field[idx][2] =  n[XX]*p.rho0u + n[YY]*p.rho0v + p.p*p.inv_c;
        char_resid_field[idx][3] = -n[XX]*p.rho0u - n[YY]*p.rho0v + p.p*p.inv_c;
        char_resid_field[idx][4] = p.inv_c * p.p;
        char_resid_field[idx][5] = (  p.rho0 * ( p.u*n[XX] + p.v*n[YY] )  );
      }

      for (Uint n=0; n<space.shape_function().nb_nodes(); ++n)
      {
        space.shape_function().compute_gradient(space.shape_function().local_coordinates().row(n),grad_sf);
        Real dSdn=0.;
        Real dSds=0.;
        Real dOmegadn=0.;
        Real dOmegads=0.;
        Real dAplusdn=0.;
        Real dAplusds=0.;
        Real dAmindn=0.;
        Real dAminds=0.;
        Real dAdn=0.;
        Real dAds=0.;
        Real domegadn=0.;
        Real domegads=0.;

        for (Uint pt=0; pt<space.shape_function().nb_nodes(); ++pt)
        {
          dSdn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][0];
          dSds -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][0];

          dOmegadn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][1];
          dOmegads -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][1];

          dAplusdn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][2];
          dAplusds -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][2];

          dAmindn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][3];
          dAminds -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][3];

          dAdn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][4];
          dAds -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][4];

          domegadn += grad_sf(0,pt) * char_field[space.connectivity()[elem][pt]][5];
          domegads -= grad_sf(1,pt) * char_field[space.connectivity()[elem][pt]][5];


        }
        dchardn[ space.connectivity()[elem][n] ][0] = dSdn;
        dchards[ space.connectivity()[elem][n] ][0] = dSds;

        dchardn[ space.connectivity()[elem][n] ][1] = dOmegadn;
        dchards[ space.connectivity()[elem][n] ][1] = dOmegads;

        dchardn[ space.connectivity()[elem][n] ][2] = dAplusdn;
        dchards[ space.connectivity()[elem][n] ][2] = dAplusds;

        dchardn[ space.connectivity()[elem][n] ][3] = dAmindn;
        dchards[ space.connectivity()[elem][n] ][3] = dAminds;

        dchardn[ space.connectivity()[elem][n] ][4] = dAdn;
        dchards[ space.connectivity()[elem][n] ][4] = dAds;

        dchardn[ space.connectivity()[elem][n] ][5] = domegadn;
        dchards[ space.connectivity()[elem][n] ][5] = domegads;

      }

    }


  }
}

////////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

