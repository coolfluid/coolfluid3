// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/OptionList.hpp"

#include "mesh/Elements.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/ElementTypes.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/Tags.hpp"

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/Tags.hpp"

#include "NavierStokesChorin.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

using namespace solver::actions::Proto;

common::ComponentBuilder < NavierStokesChorin, common::Action, LibUFEMDemo > NavierStokesChorin_builder;

// Used types for the P1 and P2 elements
typedef boost::mpl::vector1<mesh::LagrangeP1::Triag2D> LagrangeP1;
typedef boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP2::Triag2D> LagrangeP1P2;

struct CorrectionMatrix
{
  typedef void result_type;
  
  template<typename UT, typename MatrixT>
  void operator()(UT& u, MatrixT& A)
  {
    static const Uint nb_nodes = UT::EtypeT::nb_nodes;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> mass_matrix;
    mass_matrix.setZero();
    
    // 4th order quadrature
    typedef mesh::Integrators::GaussMappedCoords<4, UT::EtypeT::shape> GaussT;
    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      u.shape_function(GaussT::instance().coords.col(gauss_idx));
      const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant(GaussT::instance().coords.col(gauss_idx));
      mass_matrix.noalias() += u.shape_function().transpose() * (u.shape_function()*w);
    }
    
    for(Uint i = 0; i != UT::EtypeT::dimension; ++i)
    {
      A.template block<nb_nodes, nb_nodes>(i*nb_nodes, i*nb_nodes).noalias() = mass_matrix;
    }
  }
};

static MakeSFOp<CorrectionMatrix>::type const correction_matrix = {};

NavierStokesChorin::NavierStokesChorin ( const std::string& name ) : solver::Action( name )
{
  using boost::proto::lit;

  PhysicsConstant nu("kinematic_viscosity");

  // Manage the auxiliary velocity
  Handle<LSSActionUnsteady> auxiliary_lss = create_component<LSSActionUnsteady>("AuxiliaryLSS");
  auxiliary_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  auxiliary_lss->options().set("use_geometry_space", false);
  auxiliary_lss->set_solution_tag("navier_stokes_u_solution");

  // The auxiliary velocity field, of P2 order
  FieldVariable<0, VectorField> u("Velocity", auxiliary_lss->solution_tag(), mesh::LagrangeP2::LibLagrangeP2::library_namespace());

  // Zero the LSS
  auxiliary_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // Matrix assembly
  Handle<ProtoAction> auxiliary_mat_assembly = auxiliary_lss->create_component<ProtoAction>("MatrixAssembly");
  auxiliary_mat_assembly->set_expression(elements_expression(LagrangeP1P2(),
  group
  (
    _A( u ) = _0, _T( u ) = _0,
    element_quadrature
    (
      _A( u[_i], u[_i]) += transpose(nabla( u ))*nabla( u ),
      _T( u[_i], u[_i]) += transpose(N( u ))*N( u )
    ),
    auxiliary_lss->system_matrix += auxiliary_lss->invdt()*_T + nu*_A
  )));

  // RHS assembly
  Handle<ProtoAction> auxiliary_rhs_assembly = auxiliary_lss->create_component<ProtoAction>("RHSAssembly");
  auxiliary_rhs_assembly->set_expression(elements_expression(LagrangeP1P2(),
  group
  (
    _a[u] = _0, // Set the element vector for the u_star variable to 0 and at the same time indicate that u_star is part of the linear system
    element_quadrature
    (
      _a[u[_i]] += auxiliary_lss->invdt() * transpose(N(u))*u[_i] - transpose(N(u))*(u*nabla(u))*_col(nodal_values(u), _i)
    ),
    auxiliary_lss->system_rhs += _a
  )));
  
  Handle<LSSActionUnsteady> pressure_lss = create_component<LSSActionUnsteady>("PressureLSS");
  pressure_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  pressure_lss->set_solution_tag("navier_stokes_p_solution");
  pressure_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  
  // The pressure field. LagrangeP1 by default
  FieldVariable<1, ScalarField> p("Pressure", pressure_lss->solution_tag());
  
  Handle<ProtoAction> pressure_mat_assembly = pressure_lss->create_component<ProtoAction>("MatrixAssembly");
  pressure_mat_assembly->set_expression(elements_expression(LagrangeP1(), 
  group
  (
    _A(p) = _0,
    element_quadrature(_A(p) += transpose(nabla(p))*nabla(p)),
    pressure_lss->system_matrix += _A
  )));
  
  Handle<ProtoAction> pressure_rhs_assembly = pressure_lss->create_component<ProtoAction>("RHSAssembly");
  pressure_rhs_assembly->set_expression(elements_expression(LagrangeP1P2(),
  group
  (
    _a[p] = _0,
    element_quadrature(_a[p] += transpose(N(p))*divergence(u)),
    pressure_lss->system_rhs += -lit(pressure_lss->invdt())*_a
  )));
  
  // Velocity correction
  Handle<LSSActionUnsteady> correction_lss = create_component<LSSActionUnsteady>("CorrectionLSS");
  correction_lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  correction_lss->options().set("use_geometry_space", false);
  correction_lss->set_solution_tag(auxiliary_lss->solution_tag());
  correction_lss->create_component<math::LSS::ZeroLSS>("ZeroLSS");
  
  Handle<ProtoAction> correction_matrix_assembly = correction_lss->create_component<ProtoAction>("MatrixAssembly");
  correction_matrix_assembly->set_expression(elements_expression(LagrangeP1P2(),
  group
  (
    _A(u) = _0,
    element_quadrature(_A(u[_i], u[_i]) += transpose(N(u))*N(u)),
    //correction_matrix(u, _A),
    correction_lss->system_matrix += _A
  )));
  
  Handle<ProtoAction> correction_rhs_assembly = correction_lss->create_component<ProtoAction>("RHSAssembly");
  correction_rhs_assembly->set_expression(elements_expression(LagrangeP1P2(),
  group
  (
    _a[u] = _0,
    element_quadrature(_a[u[_i]] += transpose(N(u))*(u[_i] - lit(correction_lss->dt()) * gradient(p)[_i])),
    correction_lss->system_rhs += _a
  )));
  
  // Velocity boundary conditions
  Handle<BoundaryConditions> u_bc = create_component<BoundaryConditions>("VelocityBC");
  u_bc->mark_basic();
  u_bc->set_solution_tag(auxiliary_lss->solution_tag());
  auxiliary_lss->create_component<common::Link>("BC")->link_to(*u_bc);
  correction_lss->create_component<common::Link>("BC")->link_to(*u_bc);
  
  // Pressure boundary conditions
  Handle<BoundaryConditions> p_bc = create_component<BoundaryConditions>("PressureBC");
  p_bc->mark_basic();
  p_bc->set_solution_tag(pressure_lss->solution_tag());
  pressure_lss->create_component<common::Link>("BC")->link_to(*p_bc);
  
  // Solution of the systems
  auxiliary_lss->create_component<math::LSS::SolveLSS>("SolveLSS");
  pressure_lss->create_component<math::LSS::SolveLSS>("SolveLSS");
  correction_lss->create_component<math::LSS::SolveLSS>("SolveLSS");
  
  // Update the fields
  auxiliary_lss->create_component<ProtoAction>("Update")->set_expression(nodes_expression(u = auxiliary_lss->solution(u)));
  pressure_lss->create_component<ProtoAction>("Update")->set_expression(nodes_expression(p = pressure_lss->solution(p)));
  correction_lss->create_component<ProtoAction>("Update")->set_expression(nodes_expression(u = correction_lss->solution(u)));
}

void NavierStokesChorin::execute()
{
  using boost::proto::lit;
  Handle<common::Action> auxiliary_lss(get_child("AuxiliaryLSS"));
  Handle<common::Action> pressure_lss(get_child("PressureLSS"));
  Handle<common::Action> correction_lss(get_child("CorrectionLSS"));
  
  // Execute all LSS actions
  auxiliary_lss->execute();
  pressure_lss->execute();
  correction_lss->execute();
  
  // Disable the assembly of the matrix after the first run
  const std::vector<std::string> disabled(1, "MatrixAssembly");
  auxiliary_lss->options().set("disabled_actions", disabled);
  pressure_lss->options().set("disabled_actions", disabled);
  correction_lss->options().set("disabled_actions", disabled);
  
  // Don't zero the matrix anymore after the first run
  auxiliary_lss->get_child("ZeroLSS")->options().set("reset_matrix", false);
  pressure_lss->get_child("ZeroLSS")->options().set("reset_matrix", false);
  correction_lss->get_child("ZeroLSS")->options().set("reset_matrix", false);
}

// Make sure we set the regions on out child actions
void NavierStokesChorin::on_regions_set()
{
  get_child("AuxiliaryLSS")->options().set("regions", options()["regions"].value());
  get_child("PressureLSS")->options().set("regions", options()["regions"].value());
  get_child("CorrectionLSS")->options().set("regions", options()["regions"].value());
}

} // demo
} // UFEM
} // cf3
