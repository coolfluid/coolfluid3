// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Schemes/LF.hpp"

#include "RDM/SupportedCells.hpp" // supported cells

#include "Physics/Scalar/LinearAdv2D.hpp"       // supported physics
#include "Physics/Scalar/LinearAdvSys2D.hpp"    // supported physics
#include "Physics/Scalar/RotationAdv2D.hpp"     // supported physics

#include "RDM/Scalar/LibScalar.hpp"

using namespace cf3::common;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CellLoopT<LF, Physics::Scalar::LinearAdv2D> , RDM::CellLoop, LibScalar > LF_LinearAdv2D_Builder;

common::ComponentBuilder < CellLoopT<LF, Physics::Scalar::LinearAdvSys2D> , RDM::CellLoop, LibScalar > LF_LinearAdvSys2D_Builder;

common::ComponentBuilder < CellLoopT<LF, Physics::Scalar::RotationAdv2D> , RDM::CellLoop, LibScalar > LF_RotationAdv2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
