// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/Actions/Proto/Expression.hpp"

#include "HeatConductionSteady.hpp"
#include "LinearProblem.hpp"


namespace CF {
namespace UFEM {

using namespace Common;
using namespace Solver;
using namespace Solver::Actions::Proto;

ComponentBuilder < HeatConductionSteady, CAction, LibUFEM > HeatConductionSteady;

HeatConductionSteady::HeatConductionSteady(const std::string& name) : SteadyModel(name)
{
  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  MeshTerm<1, ScalarField> heat("Heat", "q");
  
  ConfigurableConstant<Real> k("k", "Thermal conductivity (J/(mK))", 1.);

  *this <<
  (
    problem() <<                                                                                              // The linear problem (= inner loop, but executed once here)
    problem().add_action("Assembly", elements_expression                                                      // Assembly action added to linear problem
    (
      group <<
      (
        _A = _0, _T = _0,
        element_quadrature <<
        (
          _A(temperature) += k * transpose(nabla(temperature)) * nabla(temperature),
          _T(temperature) += transpose(N(temperature))*N(temperature)
        ),
        problem().system_matrix +=  _A,
        problem().system_rhs += _T * nodal_values(heat)
      )
    ))
    << problem().boundary_conditions()                                                                        // boundary conditions
    << problem().solve_action()                                                                               // Solve the LSS
    << problem().add_action("Increment", nodes_expression(temperature = problem().solution(temperature)))     // Set the solution
  );
}



} // UFEM
} // CF
