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

#include "RDM/NavierStokes/SubsonicOutFlowWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SubsonicOutFlowWeakBc, RDM::BoundaryTerm, LibRDM > SubsonicOutFlowWeakBc_Builder;

Common::ComponentBuilder < FaceLoopT< SubsonicOutFlowWeakBc, Physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > SubsonicOutFlowWeakBc_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SubsonicOutFlowWeakBc::SubsonicOutFlowWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  std::cout << FromHere().short_str() << std::endl;

  // options

  m_options.add_option< OptionT<std::string> > ("p_out", std::string() )
      ->description("Outlet pressure (vars x,y,z)")
      ->attach_trigger ( boost::bind ( &SubsonicOutFlowWeakBc::config_pressure_function, this ) )
      ->mark_basic();

  std::cout << FromHere().short_str() << std::endl;

  pressure_function.variables("x,y,z");

  std::cout << FromHere().short_str() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void SubsonicOutFlowWeakBc::config_pressure_function()
{
  std::cout << FromHere().short_str() << std::endl;

  pressure_function.functions( m_options["p_out"].value< std::string >() );
  pressure_function.parse();

  std::cout << FromHere().short_str() << std::endl;
}


void SubsonicOutFlowWeakBc::execute()
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
