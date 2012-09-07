// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/SourceTimeInterpolation2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<SourceTimeInterpolation2D,Term,LibSDM> SourceTimeInterpolation2D_builder;

/////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
