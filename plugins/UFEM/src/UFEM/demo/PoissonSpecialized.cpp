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
#include "UFEM/NavierStokesPhysics.hpp"

#include "PoissonSpecialized.hpp"


namespace cf3 {
namespace UFEM {
namespace demo {

using namespace solver::actions::Proto;

common::ComponentBuilder < PoissonSpecialized, LSSAction, LibUFEMDemo > PoissonSpecialized_builder;

// Specialized code for triangles
struct PoissonTriagAssembly
{
  typedef void result_type;

  // Functor that takes: source term f, result element matrix A, result element RHS vector a
  // f is actually of type ETypeTVariableData, found in ElementData.hpp in the Proto directory
  template<typename FT, typename LSST>
  void operator()(const FT& f, LSST& lss, math::LSS::BlockAccumulator& acc) const
  {
    typedef mesh::LagrangeP1::Triag2D ElementT;
    // Get the coordinates of the element nodes
    const ElementT::NodesT& nodes = f.support().nodes();

    // Compute normals
    ElementT::NodesT normals;
    normals(0, XX) = nodes(1, YY) - nodes(2, YY); normals(0, YY) = nodes(2, XX) - nodes(1, XX);
    normals(1, XX) = nodes(2, YY) - nodes(0, YY); normals(1, YY) = nodes(0, XX) - nodes(2, XX);
    normals(2, XX) = nodes(0, YY) - nodes(1, YY); normals(2, YY) = nodes(1, XX) - nodes(0, XX);

    // Jacobian determinant
    const Real det_jac = normals(2, YY)*normals(1, XX) - normals(1, YY)*normals(2, XX);
    const Real c = 1. / (2.*det_jac);
    
    acc.neighbour_indices(f.support().element_connectivity());
        
    for(Uint i = 0; i != 3; ++i)
      for(Uint j = 0; j != 3; ++j)
        acc.mat(i, j) = c * (normals(i, XX)*normals(j, XX) + normals(i, YY)*normals(j, YY));

    // Get the values of the source term
    const Real f0 = f.value()[0];
    const Real f1 = f.value()[1];
    const Real f2 = f.value()[2];
    
    acc.rhs[0] = (2*f0 + f1 + f2);
    acc.rhs[1] = (f0 + 2*f1 + f2);
    acc.rhs[2] = (f0 + f1 + 2*f2);
    acc.rhs *= det_jac/24.;

    lss.matrix().add_values(acc);
    lss.rhs().add_rhs_values(acc);
  }
};

// Create an object that can be used as a function in a proto expression
static solver::actions::Proto::MakeSFOp<PoissonTriagAssembly>::type const assemble_triags = {};

PoissonSpecialized::PoissonSpecialized ( const std::string& name ) : LSSAction ( name )
{
  // Setup of private variables
  m_block_accumulator.resize(3, 1);
  
  // This determines the name of the field that will be used to store the solution
  set_solution_tag("poisson_solution");

  // Create action components that wil be executed in the order they are created here:
  // 1. Set the linear system matrix and vectors to zero
  create_component<math::LSS::ZeroLSS>("ZeroLSS");

  // 2. Assemble the system matrix and RHS using a proto expression
  Handle<ProtoAction> assembly = create_component<ProtoAction>("Assembly");

  // The unknown function. The first template argument is a constant to distinguish each variable at compile time
  FieldVariable<0, ScalarField> u("u", solution_tag());

  // The source term, to be set at runtime using an initial condition
  FieldVariable<1, ScalarField> f("f", "source_term");

  // The expression itself
  assembly->set_expression(elements_expression
  (
    boost::mpl::vector1< mesh::LagrangeP1::Triag2D>(), // Specialization for P1 triangles
    assemble_triags(f, system_matrix, m_block_accumulator)
  ));

  // 3. Apply bondary conditions
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());

  // 4. Solve the linear system
  create_component<math::LSS::SolveLSS>("SolveLSS");
  
  // 5. Update the solution
  create_component<ProtoAction>("Update")->set_expression(nodes_expression(u = solution(u)));
}

} // demo
} // UFEM
} // cf3
