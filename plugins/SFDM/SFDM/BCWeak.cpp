// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "SFDM/BCWeak.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCNull,BC,LibSFDM> bcnull_builder;

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
