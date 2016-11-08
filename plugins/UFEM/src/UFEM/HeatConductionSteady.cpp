// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

#include "HeatConductionSteady.hpp"
#include "Tags.hpp"

#include "NavierStokesPhysics.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < HeatConductionSteady, LSSAction, LibUFEM > HeatConductionSteady_builder;

// Specialized code for triangles
struct HeatSpecialized
{
  typedef void result_type;

  template<typename TempT, typename MatrixT>
  void operator()(const TempT& T, const Real& k, MatrixT& A) const
  {
    typedef mesh::LagrangeP1::Triag2D ElementT;
    const ElementT::NodesT& nodes = T.support().nodes();

    ElementT::NodesT normals;
    normals(0, XX) = nodes(1, YY) - nodes(2, YY); normals(0, YY) = nodes(2, XX) - nodes(1, XX);
    normals(1, XX) = nodes(2, YY) - nodes(0, YY); normals(1, YY) = nodes(0, XX) - nodes(2, XX);
    normals(2, XX) = nodes(0, YY) - nodes(1, YY); normals(2, YY) = nodes(1, XX) - nodes(0, XX);

    const Real c = k / (2.*(normals(2, YY)*normals(1, XX) - normals(1, YY)*normals(2, XX)));

    for(Uint i = 0; i != 3; ++i)
      for(Uint j = 0; j != 3; ++j)
        A(i, j) = c * (normals(i, XX)*normals(j, XX) + normals(i, YY)*normals(j, YY));
  }
};

static solver::actions::Proto::MakeSFOp<HeatSpecialized>::type const heat_specialized = {};

HeatConductionSteady::HeatConductionSteady ( const std::string& name ) : LSSAction ( name ), lambda_s("thermal_conductivity_solid")
{
  options().add("heat_space_name", "geometry")
    .pretty_name("Heat Space Name")
    .description("The space to use for the heat source term")
    .attach_trigger(boost::bind(&HeatConductionSteady::trigger, this));

  options().add("use_specializations", true)
    .pretty_name("Use specializations")
    .description("Use sepcialized code if available")
    .attach_trigger(boost::bind(&HeatConductionSteady::trigger, this));

  set_solution_tag("heat_conduction_solution");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  m_assembly = create_component<ProtoAction>("Assembly");

  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  create_component<math::LSS::SolveLSS>("SolveLSS");
  m_update = create_component<ProtoAction>("Update");

  trigger();

  configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
}

void HeatConductionSteady::trigger()
{
  const bool use_specializations = options().option("use_specializations").value<bool>();

  ConfigurableConstant<Real> relaxation_factor_hc("relaxation_factor_hc", "factor for relaxation in case of coupling", 1.);

  FieldVariable<0, ScalarField> T("Temperature", "heat_conduction_solution");
  FieldVariable<1, ScalarField> q("Heat", "source_terms", options().option("heat_space_name").value<std::string>());
  FieldVariable<3, ScalarField> temperature1_hc("TemperatureHistoryHC1", "temperature_history_hc");
  FieldVariable<4, ScalarField> temperature2_hc("TemperatureHistoryHC2", "temperature_history_hc");
  FieldVariable<5, ScalarField> temperature3_hc("TemperatureHistoryHC3", "temperature_history_hc");

  if(use_specializations)
  {
    boost::proto::terminal< RestrictToElementTypeTag< boost::mpl::vector5<LagrangeP1::Line1D, LagrangeP1::Tetra3D, LagrangeP1::Quad2D, LagrangeP1::Hexa3D, LagrangeP2::Line1D> > >::type generic_elements;
    boost::proto::terminal< RestrictToElementTypeTag< boost::mpl::vector1<LagrangeP1::Triag2D> > >::type specialized_elements;
    m_assembly->set_expression(elements_expression                                             // Assembly action added to linear problem
    (
      boost::mpl::vector6<LagrangeP1::Line1D, LagrangeP1::Triag2D, LagrangeP1::Tetra3D, LagrangeP1::Quad2D, LagrangeP1::Hexa3D, LagrangeP2::Line1D>(),
      group
      (
        generic_elements(
          _A = _0,
          element_quadrature
          (
            _A(T) += lambda_s * transpose(nabla(T)) * nabla(T)
          )
        ),
        specialized_elements(heat_specialized(T, lambda_s, _A(T))),
        system_matrix +=  _A,
        system_rhs += -_A * _x + integral<2>(transpose(N(T))*N(q)*jacobian_determinant) * nodal_values(q)
      )
    ));
  }
  else
  {
    m_assembly->set_expression(elements_expression                                             // Assembly action added to linear problem
    (
      boost::mpl::vector6<LagrangeP1::Line1D, LagrangeP1::Triag2D, LagrangeP1::Tetra3D, LagrangeP1::Quad2D, LagrangeP1::Hexa3D, LagrangeP2::Line1D>(),
      group
      (
        _A = _0,
        element_quadrature
        (
          _A(T) += lambda_s * transpose(nabla(T)) * nabla(T)
        ),
        system_matrix +=  _A,
        system_rhs += -_A * _x + integral<2>(transpose(N(T))*N(q)*jacobian_determinant) * nodal_values(q)
      )
    ));
  }

  m_update->set_expression(nodes_expression(T += relaxation_factor_hc*solution(T)));     // Set the solution

  // Set the proto expression for the update step
  Handle<ProtoAction>(get_child("Update"))->set_expression( nodes_expression
        (group
        (
        (T += relaxation_factor_hc*solution(T)),
        temperature3_hc = temperature2_hc,
        temperature2_hc = temperature1_hc,
        temperature1_hc = T
        ))
        );

}

void HeatConductionSteady::on_initial_conditions_set(InitialConditions& initial_conditions)

  {
  FieldVariable<0, ScalarField> T("Temperature", solution_tag());
  FieldVariable<3, ScalarField> temperature1_hc("TemperatureHistoryHC1", "temperature_history_hc");
  FieldVariable<4, ScalarField> temperature2_hc("TemperatureHistoryHC2", "temperature_history_hc");
  FieldVariable<5, ScalarField> temperature3_hc("TemperatureHistoryHC3", "temperature_history_hc");

  initial_conditions.create_initial_condition(solution_tag());

    // Use a proto action to set the temperature_history easily
  Handle<ProtoAction> temp_history_ic (initial_conditions.create_initial_condition("temperature_history", "cf3.solver.ProtoAction"));
  temp_history_ic->set_expression(nodes_expression(group(temperature1_hc = T, temperature2_hc = T, temperature3_hc = T)));

  }

} // UFEM
} // cf3
