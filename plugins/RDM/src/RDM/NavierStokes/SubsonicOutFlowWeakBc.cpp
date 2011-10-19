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

#include "RDM/NavierStokes/SubsonicOutFlowWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

using namespace cf3::common;
using namespace cf3::Mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SubsonicOutFlowWeakBc,
                           RDM::BoundaryTerm,
                           LibRDM > SubsonicOutFlowWeakBc_Builder;

common::ComponentBuilder < FaceLoopT< SubsonicOutFlowWeakBc, Physics::NavierStokes::Cons2D>,
                           RDM::FaceLoop,
                           LibRDM > SubsonicOutFlowWeakBc_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

SubsonicOutFlowWeakBc::SubsonicOutFlowWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  m_options.add_option< OptionT<std::string> > ("p_out", std::string() )
      ->description("Outlet pressure (vars x,y,z)")
      ->attach_trigger ( boost::bind ( &SubsonicOutFlowWeakBc::config_pressure_function, this ) )
      ->mark_basic();

  pressure_function.variables("x,y,z");
}


void SubsonicOutFlowWeakBc::config_pressure_function()
{
  pressure_function.functions( m_options["p_out"].value< std::string >() );
  pressure_function.parse();
}


void SubsonicOutFlowWeakBc::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    loop.select_region( region );

    loop.execute(); // loop all elements of this region
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
