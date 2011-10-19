// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "RDM/Schemes/B.hpp"

#include "RDM/SupportedCells.hpp" // supported cells

#include "Physics/Scalar/Burgers2D.hpp"       // supported physics

#include "RDM/Scalar/LibScalar.hpp"

using namespace cf3::common;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CellLoopT<B, Physics::Scalar::Burgers2D> , RDM::CellLoop, LibScalar > B_Burgers2D_Builder;

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
