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

#include "UFEM/Tags.hpp"

#include "PoissonManual.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < PoissonManual, LSSAction, LibUFEMDemo > PoissonManual_builder;

class PoissonManualAssembly : public solver::Action
{
public:
  PoissonManualAssembly ( const std::string& name ) : solver::Action(name)
  {
  }
  
  static std::string type_name () { return "PoissonManualAssembly"; }
  
  virtual void execute()
  {
    if(is_null(m_lss))
      throw common::SetupError(FromHere(), "LSS not set for " + uri().path());
    
    BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
    {
      BOOST_FOREACH(const mesh::Elements& elements, find_components_recursively_with_filter<Elements>(*region, IsElementType<mesh::LagrangeP1::Triag2D>()))
      {
        const Uint nb_elems = elements.size();
        const mesh::Connectivity& connectivity = elements.geometry_space().connectivity();
        const mesh::Field& coordinates = elements.geometry_fields().coordinates();
        Eigen::Matrix<Real, 3, 2> nodes, normals;
        math::LSS::BlockAccumulator acc;
        acc.resize(3, 1);

        Eigen::Matrix<Real, 3, 1> f;
        
        const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
        const mesh::Field& source_term = common::find_component_recursively_with_tag<mesh::Field>(mesh, "source_term");
        
        for(Uint i = 0; i != nb_elems; ++i)
        {
          acc.neighbour_indices(connectivity[i]);
          mesh::fill(nodes, coordinates, connectivity[i]);
          mesh::fill(f, source_term, connectivity[i]);
          
          normals(0, XX) = nodes(1, YY) - nodes(2, YY); normals(0, YY) = nodes(2, XX) - nodes(1, XX);
          normals(1, XX) = nodes(2, YY) - nodes(0, YY); normals(1, YY) = nodes(0, XX) - nodes(2, XX);
          normals(2, XX) = nodes(0, YY) - nodes(1, YY); normals(2, YY) = nodes(1, XX) - nodes(0, XX);
          
          // Jacobian determinant
          const Real det_jac = normals(2, YY)*normals(1, XX) - normals(1, YY)*normals(2, XX);
          const Real c = 1. / (2.*det_jac);
          
          for(Uint i = 0; i != 3; ++i)
            for(Uint j = 0; j != 3; ++j)
              acc.mat(i, j) = c * (normals(i, XX)*normals(j, XX) + normals(i, YY)*normals(j, YY));

          
          acc.rhs[0] = (2*f[0] + f[1] + f[2]);
          acc.rhs[1] = (f[0] + 2*f[1] + f[2]);
          acc.rhs[2] = (f[0] + f[1] + 2*f[2]);
          acc.rhs *= det_jac/24.;

          m_lss->add_values(acc);
        }
      }
    }
  }
  
  Handle<math::LSS::System> m_lss;
};

ComponentBuilder < PoissonManualAssembly, common::Component, LibUFEM > PoissonManualAssembly_builder;

PoissonManual::PoissonManual ( const std::string& name ) : LSSAction ( name )
{
  // This determines the name of the field that will be used to store the solution
  set_solution_tag("poisson_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using an action component
  Handle<PoissonManualAssembly> assembly = create_component<PoissonManualAssembly>("Assembly");
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

} // demo
} // UFEM
} // cf3
