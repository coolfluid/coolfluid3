// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_AdvectionDiffusion_Physics_hpp
#define CF_AdvectionDiffusion_Physics_hpp

#include "Solver/Physics.hpp"
#include "AdvectionDiffusion/LibAdvectionDiffusion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace AdvectionDiffusion {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the physics
/// @author Willem Deconinck
class AdvectionDiffusion_API Physics : public Solver::Physics {

public: // functions

  enum Vars {S=0, Vx=1, Vy=2, Vz=3};

  /// Contructor
  Physics() : Solver::Physics()
  {
    /// set 1 property variable in the physics using resize()
    resize(4);
  }

}; // Physics

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_AdvectionDiffusion_Physics_hpp
