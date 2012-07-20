// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypes.hpp"

#include "solver/Solver.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "sdm/lusgs/LUSGS.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::solver;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LUSGS, IterativeSolver, LibLUSGS > LUSGS_Builder;

////////////////////////////////////////////////////////////////////////////////

LUSGS::LUSGS ( const std::string& name ) :
  IterativeSolver(name)
{
  options().add("max_sweeps",math::Consts::uint_max()).description("Maximum number of sweeps").mark_basic();
  options().add("convergence_level",1.0e-6).description("Convergence level").mark_basic();
  options().add("recompute_jacobian_frequency",1u).description("Recompute jacobian every x iterations").mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

void LUSGS::execute()
{
  configure_option_recursively( "iterator", handle<Component>() );
  
  link_fields();

  Real convergence = math::Consts::real_max();
  Real convergence_level = options().value<Real>("convergence_level");
  Uint sweep = 0;
  Real max_sweeps = options().value<Real>("max_sweeps");
  
  while (convergence > convergence_level && sweep >= max_sweeps)
  {
    // Loop over every cell
    
      // Compute LHS, RHS of linear system
        //  Component BE
        //   - compute R
        //   - compute dR/dQ
      
      // Solve LU
    
    RealMatrix LHS; // (nb_sol_pts x nb_vars) x (nb_sol_pts x nb_vars)
    RealMatrix dQ;  // nb_sol_pts x nb_vars
    RealMatrix RHS; // nb_sol_pts x nb_vars
      
    // convergence = abs(max(dQ));
    ++sweep;
    raise_iteration_done();
  }
}

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3
