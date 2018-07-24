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

#include "GradPressureGradient.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GradPressureGradient, common::Action, LibUFEM > GradPressureGradient_builder;

////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

struct SetGradient
{
  typedef void result_type;

  template<typename GradPT, typename ValencesT, typename GradPxT, typename GradPyT>
  void operator()(const GradPT& grad_p, const ValencesT& valence, GradPxT& grad_px, GradPyT& grad_py) const
  {
    typedef typename GradPT::EtypeT ElementT;
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;

    const RealMatrix2 grad_mat = grad_p.nabla(GaussT::instance().coords.col(0))*grad_p.value();

    const Eigen::Matrix<Real, ElementT::nb_nodes, 1> inverse_valence = valence.value().array().inverse();
    grad_px.add_nodal_values_component(grad_mat(XX, XX)*inverse_valence, XX);
    grad_px.add_nodal_values_component(grad_mat(YY, XX)*inverse_valence, YY);
    grad_py.add_nodal_values_component(grad_mat(XX, YY)*inverse_valence, XX);
    grad_py.add_nodal_values_component(grad_mat(YY, YY)*inverse_valence, YY);

  }

};

static MakeSFOp<SetGradient>::type const set_gradient = {};

}

GradPressureGradient::GradPressureGradient(const std::string& name) :
  ProtoAction(name)
{
  options().add("gradp_variable", "grad_p")
    .pretty_name("gradient PressureGradient Variable")
    .description("Name for the pressure gradient variable");

  options().add("gradp_tag", "navier_stokes_gradp_solution")
    .pretty_name("GradP Tag")
    .description("Tag for the field containing the gradient of the pressure");

  options().add("gradient_name", "grad_p")
    .pretty_name("Gradient Name")
    .description("Base name for the gradient variable. Will be appended with x, y and z for the different components and prefixed with grad for the gradient and div for the divergence.");

  options().add("gradient_tag", "gradient_pressure_gradient")
    .pretty_name("Gradient Tag")
    .description("Tag for the field containing the gradients");

  options().add("initial_conditions", m_initial_conditions)
    .pretty_name("Initial Conditions")
    .description("The component that is used to manage the initial conditions in the solver this action belongs to")
    .link_to(&m_initial_conditions)
    .attach_trigger(boost::bind(&GradPressureGradient::trigger_initial_conditions, this));

  m_zero_fields = create_component<ProtoAction>("ZeroFields");
}

void GradPressureGradient::execute()
{
  m_zero_fields->execute();
  ProtoAction::execute();
}


void GradPressureGradient::on_regions_set()
{
  const Uint dim = physical_model().ndim();

  const std::string grad_tag = options().value<std::string>("gradient_tag");
  const std::string grad_base = options().value<std::string>("gradient_name");
  const std::string grad_name = "grad_" + grad_base;

  FieldVariable<0, VectorField> grad_p(options().value<std::string>("velocity_variable"), options().value<std::string>("velocity_tag"));
  FieldVariable<1, ScalarField> valence("Valence", "node_valence");

  if(dim == 2)
  {
    FieldVariable<2, VectorField> grad_px(grad_name+"x", grad_tag);
    FieldVariable<3, VectorField> grad_py(grad_name+"y", grad_tag);

    set_expression(elements_expression
    (
      boost::mpl::vector2<mesh::LagrangeP1::Triag2D, mesh::LagrangeP1::Quad2D>(),
      detail::set_gradient(grad_p, valence, grad_px, grad_py)
    ));

    m_zero_fields->set_expression(nodes_expression(group(grad_px[_i] = 0., grad_py[_i] = 0.)));
  }

  else
  {
    throw common::SetupError(FromHere(), "Unsupported dimension " + common::to_str(dim) + " for GradPressureGradient");
  }

  if(is_not_null(m_node_valence))
  {
    setup_node_valence_regions();
  }
  m_zero_fields->options().set("regions", options().option("regions").value());
}

void GradPressureGradient::trigger_initial_conditions()
{
  if(is_null(m_initial_conditions))
    return;

  if(is_null(m_node_valence))
  {
    m_node_valence = m_initial_conditions->get_child("node_valence");
    if(is_null(m_node_valence))
      m_node_valence = m_initial_conditions->create_initial_condition("node_valence", "cf3.solver.actions.NodeValence");
    setup_node_valence_regions();
  }
}

void GradPressureGradient::setup_node_valence_regions()
{
  const std::vector<common::URI> old_regions = m_node_valence->options().value< std::vector<common::URI> >("regions");
  const std::vector<common::URI> added_regions = options().value< std::vector<common::URI> >("regions");
  std::set<std::string> region_set;
  BOOST_FOREACH(const common::URI& region_uri, old_regions)
  {
    region_set.insert(region_uri.string());
  }
  BOOST_FOREACH(const common::URI& region_uri, added_regions)
  {
    region_set.insert(region_uri.string());
  }
  std::vector<common::URI> new_regions; new_regions.reserve(region_set.size());
  BOOST_FOREACH(const std::string& uri_str, region_set)
  {
    new_regions.push_back(common::URI(uri_str));
  }

  m_node_valence->options().set("regions", new_regions);
}

} // UFEM
} // cf3
