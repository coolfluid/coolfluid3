// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/ConvectiveTerm.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<LinearAdvection1D,Term,LibSFDM> linear_advection1d_builder;
common::ComponentBuilder<LinearAdvection2D,Term,LibSFDM> linear_advection2d_builder;
common::ComponentBuilder<RotationAdvection2D,Term,LibSFDM> rotation_advection2d_builder;

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
