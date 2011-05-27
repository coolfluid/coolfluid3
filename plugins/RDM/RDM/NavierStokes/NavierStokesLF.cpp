// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Schemes/CSysLF.hpp"
#include "RDM/Schemes/SchemeCSysLF.hpp"

#include "RDM/Core/SupportedCells.hpp" // supported cells

#include "RDM/Core/Euler2D.hpp"       // supported physics

#include "RDM/NavierStokes/LibNavierStokes.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoop<CSysLF,Euler2D> , RDM::ElementLoop, LibCore > CSysLF_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
