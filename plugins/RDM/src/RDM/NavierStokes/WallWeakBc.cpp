// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/FindComponents.hpp"


#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"

#include "RDM/FaceLoop.hpp"
#include "RDM/NavierStokes/WallWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < WallWeakBc, RDM::BoundaryTerm, LibRDM > WallWeakBc_Builder;

Common::ComponentBuilder < FaceLoopT< WallWeakBc, Physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > WallWeakBc_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

WallWeakBc::WallWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
}


void WallWeakBc::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {

//    std::cout << "REGION [" << region->uri().string() << "]" << std::endl;

    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
