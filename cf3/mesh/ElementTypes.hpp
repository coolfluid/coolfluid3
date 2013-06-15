// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementTypes_hpp
#define cf3_mesh_ElementTypes_hpp

#include <boost/mpl/joint_view.hpp>

#include "mesh/ElementTypePredicates.hpp"
#include "mesh/LagrangeP0/ElementTypes.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP2/ElementTypes.hpp"
#include "mesh/LagrangeP3/ElementTypes.hpp"

#include "mesh/ElementTypeT.hpp"

namespace cf3 {
namespace mesh {

///////////////////////////////////////////////////////////////////////////////

/// List of all supported element types
typedef boost::mpl::joint_view< LagrangeP0::ElementTypes,
        boost::mpl::joint_view< LagrangeP1::ElementTypes,
        boost::mpl::joint_view< LagrangeP2::ElementTypes,
                                LagrangeP3::ElementTypes
  > > > ElementTypes;

typedef boost::mpl::joint_view< LagrangeP0::CellTypes,
        boost::mpl::joint_view< LagrangeP1::CellTypes,
        boost::mpl::joint_view< LagrangeP2::CellTypes,
                                LagrangeP3::CellTypes
  > > > CellTypes;

typedef boost::mpl::joint_view< LagrangeP0::FaceTypes,
        boost::mpl::joint_view< LagrangeP1::FaceTypes,
        boost::mpl::joint_view< LagrangeP2::FaceTypes,
                                LagrangeP3::FaceTypes
  > > > FaceTypes;

typedef boost::mpl::joint_view< LagrangeP0::EdgeTypes,
        boost::mpl::joint_view< LagrangeP1::EdgeTypes,
        boost::mpl::joint_view< LagrangeP2::EdgeTypes,
                                LagrangeP3::EdgeTypes
  > > > EdgeTypes;

///////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_ElementTypes_hpp
