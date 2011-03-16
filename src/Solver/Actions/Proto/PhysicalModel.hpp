// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_PhysicalModel_hpp
#define CF_Solver_Actions_Proto_PhysicalModel_hpp

#include "Common/CF.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Simplified version of a physical model, intended for internal use by proto
struct PhysicalModel
{
  PhysicalModel() : nb_dofs(0) {}
  
  /// Number of degrees of freedom (i.e. 3 for u, v, p)
  Uint nb_dofs;
  
  /// Offset of each variable, i.e. in V (vector of u and v) and p, V has offset 0, and p has offset 2 when the order is uvp in the global system
  std::map<std::string, Uint> variable_offsets;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_PhysicalModel_hpp
