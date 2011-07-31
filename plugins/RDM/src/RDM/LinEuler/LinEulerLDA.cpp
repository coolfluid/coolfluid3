// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Schemes/RKLDA.hpp"

#include "RDM/SupportedCells.hpp" // supported cells

#include "Physics/LinEuler/Cons2D.hpp"       // supported physics

#include "LibLinEuler.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoopT<RKLDA,Physics::LinEuler::Cons2D> ,
                           RDM::CellLoop,
                           LibLinEuler >
                           LDA_Cons2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
