// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/GPU/CellLoopGPU.hpp"

#include "RDM/GPU/CSysLDAGPU.hpp"
#include "RDM/GPU/SchemeCSysLDAGPU.hpp"

#include "Physics/Scalar/RotationAdv2D.hpp"     // supported physics
#include "Physics/NavierStokes/Cons2D.hpp"      // supported physics

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoopGPU<CSysLDAGPU,Physics::Scalar::RotationAdv2D>,
                           RDM::CellLoop,
                           LibRDM >
                           CSysLDAGPU_LinearAdv2D_Builder;

Common::ComponentBuilder < CellLoopGPU<CSysLDAGPU,Physics::NavierStokes::Cons2D>,
                           RDM::CellLoop,
                           LibRDM >
                           CSysLDAGPU_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
