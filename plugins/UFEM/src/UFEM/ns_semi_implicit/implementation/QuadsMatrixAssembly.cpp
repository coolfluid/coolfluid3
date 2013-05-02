// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <coolfluid-ufem-config.hpp>

#include "../MatrixAssembly.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

void NavierStokesSemiImplicit::set_elements_expressions_quad()
{
#ifdef CF3_UFEM_ENABLE_QUADS
  set_elements_expressions< boost::mpl::vector1<mesh::LagrangeP1::Quad2D> >("Quads");
#endif
}

} // UFEM
} // cf3
