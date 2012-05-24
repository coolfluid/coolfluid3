// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/scalar/Diffusion1D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace scalar {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<Diffusion1D,Term,LibScalar> Diffusion1D_builder;

/////////////////////////////////////////////////////////////////////////////

} // scalar
} // sdm
} // cf3
