// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionArray.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"

#include "RDM/WeakDirichlet.hpp"

#include "Physics/Scalar/LinearAdv2D.hpp"    // to remove
#include "Physics/Scalar/LinearAdv3D.hpp"    // to remove
#include "Physics/Scalar/LinearAdvSys2D.hpp" // to remove
#include "Physics/Scalar/RotationAdv2D.hpp"  // to remove
#include "Physics/Scalar/Burgers2D.hpp"      // to remove
#include "Physics/NavierStokes/Cons2D.hpp"   // to remove

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WeakDirichlet, RDM::BoundaryTerm, LibRDM > WeakDirichlet_Builder;

common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdv2D>    , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdv2D_Builder;
//common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdv3D>    , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdv3D_Builder;
common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdvSys2D> , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdvSys2D_Builder;
common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::RotationAdv2D>  , RDM::FaceLoop, LibRDM > WeakDirichlet_RotationAdv2D_Builder;
common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::Burgers2D>      , RDM::FaceLoop, LibRDM > WeakDirichlet_Burgers2D_Builder;
common::ComponentBuilder < FaceLoopT< WeakDirichlet, NavierStokes::Cons2D>   , RDM::FaceLoop, LibRDM > WeakDirichlet_Cons2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WeakDirichlet::WeakDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  options().add("functions", std::vector<std::string>())
      .description("math function applied as Dirichlet boundary condition (vars x,y)")
      .attach_trigger ( boost::bind ( &WeakDirichlet::config_function, this ) )
      .mark_basic();

  function.variables("x,y,z");
}


void WeakDirichlet::config_function()
{
  function.functions( options()["functions"].value<std::vector<std::string> >() );
  function.parse();
}


void WeakDirichlet::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Handle< mesh::Region >& region, m_loop_regions)
  {

//    std::cout << "REGION [" << region->uri().string() << "]" << std::endl;

    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
