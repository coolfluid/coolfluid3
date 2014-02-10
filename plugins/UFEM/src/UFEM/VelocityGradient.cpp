// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Component.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/PropertyList.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/actions/Iterate.hpp"
#include "solver/actions/NodeValence.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "VelocityGradient.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < VelocityGradient, common::Action, LibUFEM > VelocityGradient_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

struct SetGradient
{
  typedef void result_type;
  
  template<typename UT, typename ValencesT, typename GradUxT, typename GradUyT>
  void operator()(const UT& u, const ValencesT& valence, GradUxT& grad_ux, GradUyT& grad_uy) const
  {
    typedef typename UT::EtypeT ElementT;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;

    const RealMatrix2 grad_mat = u.nabla(GaussT::instance().coords.col(0))*u.value();

    const Eigen::Matrix<Real, ElementT::nb_nodes, 1> inverse_valence = valence.value().array().inverse();
    grad_ux.add_nodal_values_component(grad_mat(XX, XX)*inverse_valence, XX);
    grad_ux.add_nodal_values_component(grad_mat(YY, XX)*inverse_valence, YY);
    grad_uy.add_nodal_values_component(grad_mat(XX, YY)*inverse_valence, XX);
    grad_uy.add_nodal_values_component(grad_mat(YY, YY)*inverse_valence, YY);
  }
  
  template<typename UT, typename ValencesT, typename GradUxT, typename GradUyT, typename GradUzT>
  void operator()(const UT& u, const ValencesT& valence, GradUxT& grad_ux, GradUyT& grad_uy, GradUzT& grad_uz) const
  {
    typedef typename UT::EtypeT ElementT;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;

    const RealMatrix3 grad_mat = u.nabla(GaussT::instance().coords.col(0))*u.value();

    const Eigen::Matrix<Real, ElementT::nb_nodes, 1> inverse_valence = valence.value().array().inverse();
    
    grad_ux.add_nodal_values_component(grad_mat(XX, XX)*inverse_valence, XX);
    grad_ux.add_nodal_values_component(grad_mat(YY, XX)*inverse_valence, YY);
    grad_ux.add_nodal_values_component(grad_mat(ZZ, XX)*inverse_valence, ZZ);
    
    grad_uy.add_nodal_values_component(grad_mat(XX, YY)*inverse_valence, XX);
    grad_uy.add_nodal_values_component(grad_mat(YY, YY)*inverse_valence, YY);
    grad_uy.add_nodal_values_component(grad_mat(ZZ, YY)*inverse_valence, ZZ);
    
    grad_uz.add_nodal_values_component(grad_mat(XX, ZZ)*inverse_valence, XX);
    grad_uz.add_nodal_values_component(grad_mat(YY, ZZ)*inverse_valence, YY);
    grad_uz.add_nodal_values_component(grad_mat(ZZ, ZZ)*inverse_valence, ZZ);
  }
};

static MakeSFOp<SetGradient>::type const set_gradient = {};

}

VelocityGradient::VelocityGradient(const std::string& name) :
  ProtoAction(name)
{ 
  options().add("velocity_variable", "Velocity")
    .pretty_name("Velocity Variable")
    .description("Name for the velocity variable");
  
  options().add("velocity_tag", "navier_stokes_u_solution")
    .pretty_name("Velocity Tag")
    .description("Tag for the field containing the velocity");
    
  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&VelocityGradient::trigger_initial_conditions, this));
    
  m_zero_fields = create_component<ProtoAction>("ZeroFields");
}

void VelocityGradient::execute()
{
  m_zero_fields->execute();
  ProtoAction::execute();
}


void VelocityGradient::on_regions_set()
{
  const Uint dim = physical_model().ndim();

  FieldVariable<0, VectorField> u(options().value<std::string>("velocity_variable"), options().value<std::string>("velocity_tag"));
  FieldVariable<1, ScalarField> valence("Valence", "node_valence");

  const std::string grad_tag = "velocity_gradient";

  if(dim == 2)
  {
    FieldVariable<2, VectorField> grad_ux("grad_ux", grad_tag);
    FieldVariable<3, VectorField> grad_uy("grad_uy", grad_tag);
    
    set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Quad2D>(),
      detail::set_gradient(u, valence, grad_ux, grad_uy)
    ));
    
    m_zero_fields->set_expression(nodes_expression(group(grad_ux[_i] = 0., grad_uy[_i] = 0.)));
  }
  else if(dim == 3)
  {
    FieldVariable<2, VectorField> grad_ux("grad_ux", grad_tag);
    FieldVariable<3, VectorField> grad_uy("grad_uy", grad_tag);
    FieldVariable<4, VectorField> grad_uz("grad_uz", grad_tag);
    
    set_expression(elements_expression
    (
      boost::mpl::vector3<mesh::LagrangeP1::Tetra3D, mesh::LagrangeP1::Hexa3D, mesh::LagrangeP1::Prism3D>(),
      detail::set_gradient(u, valence, grad_ux, grad_uy, grad_uz)
    ));
    
    m_zero_fields->set_expression(nodes_expression(group(grad_ux[_i] = 0., grad_uy[_i] = 0., grad_uz[_i] = 0.)));
  }
  else
  {
    throw common::SetupError(FromHere(), "Unsupported dimension " + common::to_str(dim) + " for VelocityGradient");
  }

  if(is_not_null(m_node_valence))
  {
    m_node_valence->options().set("regions", options().option("regions").value());
  }
  m_zero_fields->options().set("regions", options().option("regions").value());
}


void VelocityGradient::trigger_initial_conditions()
{
  if(is_null(m_initial_conditions))
    return;

  if(is_null(m_node_valence))
  {
    m_node_valence = m_initial_conditions->create_initial_condition("node_valence", "cf3.solver.actions.NodeValence");
    on_regions_set();
  }
}

} // UFEM
} // cf3
