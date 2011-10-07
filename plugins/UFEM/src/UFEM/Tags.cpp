// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UFEM/Tags.hpp"

namespace CF {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////

const char * Tags::solution() { return "solution"; }
const char * Tags::source_terms() { return "source_terms"; }
const char * Tags::coefficients() { return "coefficients"; }

////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF
