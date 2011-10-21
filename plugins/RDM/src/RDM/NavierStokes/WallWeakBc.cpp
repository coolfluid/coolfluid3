// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/FindComponents.hpp"


#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"

#include "RDM/FaceLoop.hpp"
#include "RDM/NavierStokes/WallWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WallWeakBc, RDM::BoundaryTerm, LibRDM > WallWeakBc_Builder;

common::ComponentBuilder < FaceLoopT< WallWeakBc, Physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > WallWeakBc_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

WallWeakBc::WallWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
}


void WallWeakBc::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(mesh::Region::Ptr& region, m_loop_regions)
  {

//    std::cout << "REGION [" << region->uri().string() << "]" << std::endl;

    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
