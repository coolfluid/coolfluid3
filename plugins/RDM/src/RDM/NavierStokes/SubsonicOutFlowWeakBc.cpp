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

#include "RDM/NavierStokes/SubsonicOutFlowWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SubsonicOutFlowWeakBc,
                           RDM::BoundaryTerm,
                           LibRDM > SubsonicOutFlowWeakBc_Builder;

common::ComponentBuilder < FaceLoopT< SubsonicOutFlowWeakBc, physics::NavierStokes::Cons2D>,
                           RDM::FaceLoop,
                           LibRDM > SubsonicOutFlowWeakBc_Euler2D_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

SubsonicOutFlowWeakBc::SubsonicOutFlowWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  options().add("p_out", std::string() )
      .description("Outlet pressure (vars x,y,z)")
      .attach_trigger ( boost::bind ( &SubsonicOutFlowWeakBc::config_pressure_function, this ) )
      .mark_basic();

  pressure_function.variables("x,y,z");
}


void SubsonicOutFlowWeakBc::config_pressure_function()
{
  pressure_function.functions( options()["p_out"].value< std::string >() );
  pressure_function.parse();
}


void SubsonicOutFlowWeakBc::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Handle< mesh::Region >& region, m_loop_regions)
  {
    loop.select_region( region );

    loop.execute(); // loop all elements of this region
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
