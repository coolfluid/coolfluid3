// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_SupportedTypes_hpp
#define CF_RDM_SupportedTypes_hpp

#include <boost/mpl/vector.hpp>

#include "Triag2DLagrangeP1.hpp"
#include "Triag2DLagrangeP2.hpp"
#include "Triag2DLagrangeP2B.hpp"
#include "Triag2DLagrangeP3.hpp"

#include "Quad2DLagrangeP1.hpp"
#include "Quad2DLagrangeP2.hpp"

namespace CF {
namespace RDM {

/// List of all supported shapefunctions
typedef boost::mpl::vector<
Triag2DLagrangeP1,
Triag2DLagrangeP2,
Triag2DLagrangeP2B,
Triag2DLagrangeP3,
Quad2DLagrangeP1,
Quad2DLagrangeP2,
> CellTypes;

} // RDM
} // CF

#endif // CF_RDM_SupportedTypes_hpp
