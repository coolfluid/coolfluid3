// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ElementTypes_hpp
#define CF_Mesh_ElementTypes_hpp

#include <boost/mpl/joint_view.hpp>

#include "Mesh/ElementTypePredicates.hpp"
#include "Mesh/LagrangeP0/ElementTypes.hpp"
#include "Mesh/LagrangeP1/ElementTypes.hpp"
#include "Mesh/LagrangeP2/ElementTypes.hpp"
#include "Mesh/LagrangeP3/ElementTypes.hpp"
#include "Mesh/LagrangeP2B/ElementTypes.hpp"

#include "Mesh/ElementTypeT.hpp"

namespace CF {
namespace Mesh {

///////////////////////////////////////////////////////////////////////////////

/// List of all supported element types
typedef boost::mpl::joint_view< LagrangeP0::ElementTypes,
        boost::mpl::joint_view< LagrangeP1::ElementTypes,
        boost::mpl::joint_view< LagrangeP2::ElementTypes,
        boost::mpl::joint_view< LagrangeP3::ElementTypes,
                                LagrangeP2B::ElementTypes
  > > > > ElementTypes;

typedef boost::mpl::joint_view< LagrangeP0::CellTypes,
        boost::mpl::joint_view< LagrangeP1::CellTypes,
        boost::mpl::joint_view< LagrangeP2::CellTypes,
        boost::mpl::joint_view< LagrangeP3::CellTypes,
                                LagrangeP2B::CellTypes
  > > > > CellTypes;

typedef boost::mpl::joint_view< LagrangeP0::FaceTypes,
        boost::mpl::joint_view< LagrangeP1::FaceTypes,
        boost::mpl::joint_view< LagrangeP2::FaceTypes,
        boost::mpl::joint_view< LagrangeP3::FaceTypes,
                                LagrangeP2B::FaceTypes
  > > > > FaceTypes;

typedef boost::mpl::joint_view< LagrangeP0::EdgeTypes,
        boost::mpl::joint_view< LagrangeP1::EdgeTypes,
        boost::mpl::joint_view< LagrangeP2::EdgeTypes,
        boost::mpl::joint_view< LagrangeP3::EdgeTypes,
                                LagrangeP2B::EdgeTypes
  > > > > EdgeTypes;

///////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

#endif // CF_Mesh_ElementTypes_hpp
