// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/ShapeFunction.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

#include "HeatConductionVirtual.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < HeatConductionVirtual, LSSAction, LibUFEM > HeatConductionVirtual_builder;

class HeatConductionVirtualAssembly : public solver::Action
{
public:
  HeatConductionVirtualAssembly ( const std::string& name ) : solver::Action(name)
  {
  }

  static std::string type_name () { return "HeatConductionVirtualAssembly"; }

  virtual void execute()
  {
    typedef mesh::Integrators::GaussMappedCoords<2, GeoShape::TRIAG> GaussT;


    if(is_null(m_lss))
      throw common::SetupError(FromHere(), "LSS not set for " + uri().path());

    BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
    {
      BOOST_FOREACH(const mesh::Elements& elements, find_components_recursively_with_filter<Elements>(*region, IsElementType<mesh::LagrangeP1::Triag2D>()))
      {
        const Uint nb_elems = elements.size();
        const mesh::Connectivity& connectivity = elements.geometry_space().connectivity();
        const mesh::ShapeFunction& sf = elements.geometry_space().shape_function();
        const mesh::ElementType& et = elements.element_type();
        const mesh::Field& coordinates = elements.geometry_fields().coordinates();
        math::LSS::BlockAccumulator acc;
        const Uint nb_nodes = sf.nb_nodes();
        acc.resize(nb_nodes, 1);
        RealMatrix jacobian_adj;
        jacobian_adj.resize(sf.dimensionality(), sf.dimensionality());
        RealMatrix nodes;
        nodes.resize(nb_nodes, sf.dimensionality());
        RealMatrix nabla;
        nabla.resize(sf.dimensionality(), nb_nodes);
        
        std::vector<RealMatrix> gauss_gradients(GaussT::nb_points, RealMatrix(sf.dimensionality(), nb_nodes));
        for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
        {
          sf.compute_gradient(GaussT::instance().coords.col(gauss_idx), gauss_gradients[gauss_idx]);
        }

        for(Uint i = 0; i != nb_elems; ++i)
        {
          acc.neighbour_indices(connectivity[i]);
          mesh::fill(nodes, coordinates, connectivity[i]);
          acc.reset();

          for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
          {
            et.compute_jacobian_adjoint(GaussT::instance().coords.col(gauss_idx), nodes, jacobian_adj);
            nabla.noalias() = jacobian_adj*gauss_gradients[gauss_idx];
            acc.mat.noalias() += GaussT::instance().weights[gauss_idx] / et.jacobian_determinant(GaussT::instance().coords.col(gauss_idx), nodes) * nabla.transpose()*nabla;
          }

          m_lss->add_values(acc);
        }
      }
    }
  }

  Handle<math::LSS::System> m_lss;
};

ComponentBuilder < HeatConductionVirtualAssembly, common::Component, LibUFEM > HeatConductionVirtualAssembly_builder;

HeatConductionVirtual::HeatConductionVirtual ( const std::string& name ) : LSSAction ( name )
{
  set_solution_tag("heat_conduction_solution");

  ConfigurableConstant<Real> k("k", "Thermal conductivity (J/(mK))", 1.);
  FieldVariable<0, ScalarField> T("Temperature", "heat_conduction_solution");

  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  Handle<HeatConductionVirtualAssembly> assembly = create_component<HeatConductionVirtualAssembly>("Assembly");

  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("SetSolution")->set_expression(nodes_expression(T += solution(T)));

  configure_option_recursively(solver::Tags::physical_model(), m_physical_model);

  options().option("lss").link_to(&assembly->m_lss);
}

void HeatConductionVirtual::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
