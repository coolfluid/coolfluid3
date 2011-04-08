// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/CSysLDA.hpp"
#include "RDM/SchemeCSysLDA.hpp"

#include "RDM/SupportedTypes.hpp"    // supported elements

#include "RDM/Euler2D.hpp"       // supported physics

using namespace CF::Common;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementLoop<CSysLDA,Euler2D> , RDM::DomainLoop, LibRDM > CSysLDA_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
