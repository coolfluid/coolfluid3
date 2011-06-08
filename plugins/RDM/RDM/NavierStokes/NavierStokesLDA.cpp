// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Schemes/CSysLDA.hpp"
#include "RDM/Schemes/SchemeCSysLDA.hpp"

#include "RDM/Core/SupportedCells.hpp" // supported cells

#include "RDM/Core/Euler2D.hpp"       // supported physics

#include "RDM/NavierStokes/LibNavierStokes.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoopT<CSysLDA,Euler2D> , RDM::CellLoop, LibNavierStokes > CSysLDA_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
