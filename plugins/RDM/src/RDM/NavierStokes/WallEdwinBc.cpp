// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/FindComponents.hpp"


#include "Mesh/CRegion.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"

#include "RDM/FaceLoop.hpp"

#include "RDM/NavierStokes/WallEdwinBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::Mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WallEdwinBc, RDM::BoundaryTerm, LibRDM > WallEdwinBc_Builder;

common::ComponentBuilder < FaceLoopT< WallEdwinBc, Physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > WallEdwinBc_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WallEdwinBc::WallEdwinBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);
}


/////////////////////////////////////////////////////////////////////////////////////

void WallEdwinBc::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

////////////////////////////////////////////////////////////////////////////////////

