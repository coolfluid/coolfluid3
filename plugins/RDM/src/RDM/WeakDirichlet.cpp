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

#include "RDM/WeakDirichlet.hpp"

#include "Physics/Scalar/LinearAdv2D.hpp"    // to remove
#include "Physics/Scalar/LinearAdv3D.hpp"    // to remove
#include "Physics/Scalar/LinearAdvSys2D.hpp" // to remove
#include "Physics/Scalar/RotationAdv2D.hpp"  // to remove
#include "Physics/Scalar/Burgers2D.hpp"      // to remove
#include "Physics/NavierStokes/Cons2D.hpp"   // to remove

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < WeakDirichlet, RDM::BoundaryTerm, LibRDM > WeakDirichlet_Builder;

Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdv2D>    , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdv2D_Builder;
//Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdv3D>    , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdv3D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::LinearAdvSys2D> , RDM::FaceLoop, LibRDM > WeakDirichlet_LinearAdvSys2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::RotationAdv2D>  , RDM::FaceLoop, LibRDM > WeakDirichlet_RotationAdv2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Scalar::Burgers2D>      , RDM::FaceLoop, LibRDM > WeakDirichlet_Burgers2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, NavierStokes::Cons2D>   , RDM::FaceLoop, LibRDM > WeakDirichlet_Cons2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WeakDirichlet::WeakDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->description("Math function applied as Dirichlet boundary condition (vars x,y)")
      ->attach_trigger ( boost::bind ( &WeakDirichlet::config_function, this ) )
      ->mark_basic();

  function.variables("x,y,z");
}


void WeakDirichlet::config_function()
{
  function.functions( m_options["functions"].value<std::vector<std::string> >() );
  function.parse();
}


void WeakDirichlet::execute()
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

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
