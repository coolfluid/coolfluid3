// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Schemes/CSysLDA.hpp"
#include "RDM/Schemes/SchemeCSysLDA.hpp"

#include "RDM/Core/SupportedCells.hpp" // supported cells

#include "RDM/Core/LinearAdv2D.hpp"       // supported physics
#include "RDM/Core/LinearAdvSys2D.hpp"    // supported physics
#include "RDM/Core/RotationAdv2D.hpp"     // supported physics

#include "RDM/Scalar/LibScalar.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoopT<CSysLDA,LinearAdv2D>,
                           RDM::CellLoop,
                           LibScalar > CSysLDA_LinearAdv2D_Builder;

Common::ComponentBuilder < CellLoopT<CSysLDA,LinearAdvSys2D>,
                           RDM::CellLoop,
                           LibScalar > CSysLDA_LinearAdvSys2D_Builder;

Common::ComponentBuilder < CellLoopT<CSysLDA,RotationAdv2D>,
                           RDM::CellLoop,
                           LibScalar > CSysLDA_RotationAdv2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
