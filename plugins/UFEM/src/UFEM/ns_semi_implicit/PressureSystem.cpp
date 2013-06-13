// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <Epetra_CrsMatrix.h>
#include <EpetraExt_MatrixMatrix.h>

#include <Thyra_describeLinearOp.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

#include "math/LSS/System.hpp"
#include "math/LSS/Trilinos/ThyraOperator.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/Action.hpp"
#include "solver/Tags.hpp"
#include "solver/actions/Proto/ElementLooper.hpp"

#include "../SUPG.hpp"

#include "PressureSystem.hpp"

#include <coolfluid-ufem-config.hpp>

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < PressureSystem, common::ActionDirector, LibUFEM > PressureSystem_Builder;

PressureSystem::PressureSystem(const std::string& name) :
  LSSActionUnsteady(name)
{
}

PressureSystem::~PressureSystem()
{
}

void PressureSystem::do_create_lss(common::PE::CommPattern &cp, const math::VariablesDescriptor &vars, std::vector<Uint> &node_connectivity, std::vector<Uint> &starting_indices, const std::vector<Uint> &periodic_links_nodes, const std::vector<bool> &periodic_links_active)
{
  // Due to simplification, no special sparsity is needed for the pressure LSS
  LSSActionUnsteady::do_create_lss(cp, vars, node_connectivity, starting_indices, periodic_links_nodes, periodic_links_active);
}

class PressureSystemAssembly : public solver::Action
{
public:
  PressureSystemAssembly(const std::string& name) :
    solver::Action(name),
    u("Velocity", "navier_stokes_u_solution"),
    p("Pressure", "navier_stokes_p_solution"),
    u_adv("AdvectionVelocity", "linearized_velocity"),
    nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
    theta(0.5)
  {
    options().add("pressure_lss_action", m_pressure_lss_action)
      .pretty_name("Pressure LSS Action")
      .description("LSS action for the pressure system")
      .link_to(&m_pressure_lss_action);

    options().add("theta", theta)
      .pretty_name("Theta")
      .description("Theta parameter for the theta scheme")
      .link_to(&theta);
  }

  static std::string type_name () { return "PressureSystemAssembly"; }

  virtual void execute()
  {
    if(is_null(m_pressure_lss_action))
      throw common::SetupError(FromHere(), "pressure_lss_action not set for PressureSystemAssembly " + uri().string());

    Handle<math::LSS::System> p_lss(m_pressure_lss_action->get_child("LSS"));
    cf3_assert(is_not_null(p_lss));

    p_lss->reset(0.);

    // Assemble simplified pressure matrix
#ifdef CF3_UFEM_ENABLE_TRIAGS
    assemble_pp<mesh::LagrangeP1::Triag2D>(m_pressure_lss_action->system_matrix);
#endif
#ifdef CF3_UFEM_ENABLE_QUADS
    assemble_pp<mesh::LagrangeP1::Quad2D>(m_pressure_lss_action->system_matrix);
#endif
#ifdef CF3_UFEM_ENABLE_TETRAS
    assemble_pp<mesh::LagrangeP1::Tetra3D>(m_pressure_lss_action->system_matrix);
#endif
#ifdef CF3_UFEM_ENABLE_HEXAS
    assemble_pp<mesh::LagrangeP1::Hexa3D>(m_pressure_lss_action->system_matrix);
#endif
#ifdef CF3_UFEM_ENABLE_PRISMS
    assemble_pp<mesh::LagrangeP1::Prism3D>(m_pressure_lss_action->system_matrix);
#endif
  }

  template<typename ElementT>
  void assemble_pp(const SystemMatrix& mat)
  {
    const Real u_ref = physical_model().options().value<Real>("reference_velocity");
    for_each_element<ElementT>(group
    (
      _A = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature( _A(p, p) += transpose(nabla(p)) * nabla(p) ),
      mat += lit(theta) * (lit(tau_ps) + lit(m_pressure_lss_action->dt())) *_A
    ));
  }

private:
  // Helper function to loop over all regions
  template<typename ElementT, typename ExprT>
  void for_each_element(const ExprT& expr)
  {
    BOOST_FOREACH(const Handle<mesh::Region> region, m_loop_regions)
    {
      solver::actions::Proto::for_each_element< boost::mpl::vector1<ElementT> >(*region, expr);
    }
  }

  Handle<LSSActionUnsteady> m_pressure_lss_action;

  /// The velocity solution field
  FieldVariable<0, VectorField> u;
  /// The pressure solution field
  FieldVariable<1, ScalarField> p;
  /// The linearized advection velocity
  FieldVariable<2, VectorField> u_adv;
  /// Effective viscosity field
  FieldVariable<3, ScalarField> nu_eff;

  Real tau_ps, tau_su, tau_bulk;
  Real theta;
};

common::ComponentBuilder < PressureSystemAssembly, common::Action, LibUFEM > PressureSystemAssembly_Builder;

} // UFEM
} // cf3
