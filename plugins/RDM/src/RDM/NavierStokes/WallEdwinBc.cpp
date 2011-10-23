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

#include "RDM/NavierStokes/WallEdwinBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

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

  boost_foreach(mesh::Region::Ptr& region, m_loop_regions)
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

