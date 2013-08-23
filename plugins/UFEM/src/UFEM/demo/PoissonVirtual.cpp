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

#include "UFEM/Tags.hpp"

#include "PoissonVirtual.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < PoissonVirtual, LSSAction, LibUFEMDemo > PoissonVirtual_builder;

class PoissonVirtualAssembly : public solver::Action
{
public:
  PoissonVirtualAssembly ( const std::string& name ) : solver::Action(name)
  {
  }

  static std::string type_name () { return "PoissonVirtualAssembly"; }

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
        RealRowVector N;
        N.resize(sf.nb_nodes());
        RealColVector f;
        f.resize(sf.nb_nodes());
        RealMatrix nodes;
        nodes.resize(nb_nodes, sf.dimensionality());
        RealMatrix nabla;
        nabla.resize(sf.dimensionality(), nb_nodes);

        const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
        const mesh::Field& source_term = common::find_component_recursively_with_tag<mesh::Field>(mesh, "source_term");
        
        std::vector<RealMatrix> gauss_gradients(GaussT::nb_points, RealMatrix(sf.dimensionality(), nb_nodes));
        for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
        {
          sf.compute_gradient(GaussT::instance().coords.col(gauss_idx), gauss_gradients[gauss_idx]);
        }

        for(Uint i = 0; i != nb_elems; ++i)
        {
          acc.neighbour_indices(connectivity[i]);
          mesh::fill(nodes, coordinates, connectivity[i]);
          for(Uint j = 0; j != nb_nodes; ++j)
            f[j] = source_term[acc.indices[j]][0];

          acc.reset();

          for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
          {
            et.compute_jacobian_adjoint(GaussT::instance().coords.col(gauss_idx), nodes, jacobian_adj);
            const Real w = GaussT::instance().weights[gauss_idx];
            const Real det_jac =  et.jacobian_determinant(GaussT::instance().coords.col(gauss_idx), nodes);
            sf.compute_value(GaussT::instance().coords.col(gauss_idx), N);
            nabla.noalias() = jacobian_adj*gauss_gradients[gauss_idx];
            acc.mat.noalias() += w * nabla.transpose()*nabla / det_jac;
            acc.rhs.noalias() += w*det_jac*N.transpose()*(N*f);
          }
          
          m_lss->add_values(acc);
        }
      }
    }
  }

  Handle<math::LSS::System> m_lss;
};

ComponentBuilder < PoissonVirtualAssembly, common::Component, LibUFEM > PoissonVirtualAssembly_builder;

PoissonVirtual::PoissonVirtual ( const std::string& name ) : LSSAction ( name )
{
  // This determines the name of the field that will be used to store the solution
  set_solution_tag("poisson_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using an action component
  Handle<PoissonVirtualAssembly> assembly = create_component<PoissonVirtualAssembly>("Assembly");
  options().option("lss").link_to(&assembly->m_lss);

  // 3. Apply bondary conditions
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // 4. Solve the linear system
  create_component<math::LSS::SolveLSS>("SolveLSS");

  // 5. Update the solution
  // The unknown function. The first template argument is a constant to distinguish each variable at compile time
  FieldVariable<0, ScalarField> u("u", solution_tag());
  // The source term, to be set at runtime using an initial condition
  FieldVariable<1, ScalarField> f("f", "source_term");
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(u = solution(u)));
  // Dummy action to have f created automatically
  create_component<ProtoAction>("DummyF")->set_expression(nodes_expression(f));
}

void PoissonVirtual::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}

} // demo
} // UFEM
} // cf3
