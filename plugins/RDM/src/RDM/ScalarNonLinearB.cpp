// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/CSysB.hpp"
#include "RDM/SchemeCSysB.hpp"

#include "RDM/SupportedTypes.hpp"    // supported elements

#include "RDM/Burgers2D.hpp"       // supported physics

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CellLoop<CSysB,Burgers2D> , RDM::ElementLoop, LibRDM > CSysB_Burgers2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
