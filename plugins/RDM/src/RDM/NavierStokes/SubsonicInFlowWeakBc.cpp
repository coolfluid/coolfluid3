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

#include "RDM/NavierStokes/SubsonicInFlowWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"


using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SubsonicInFlowWeakBc, RDM::BoundaryTerm, LibRDM > SubsonicInFlowWeakBc_Builder;

common::ComponentBuilder < FaceLoopT< SubsonicInFlowWeakBc, physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > SubsonicInFlowWeakBc_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SubsonicInFlowWeakBc::SubsonicInFlowWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  options().add("rho_in", std::string() )
      .description("Inlet density (vars x,y,z)")
      .attach_trigger ( boost::bind ( &SubsonicInFlowWeakBc::config_density_function, this ) )
      .mark_basic();

  density_function.variables("x,y,z");

  options().add("vel_in",std::vector<std::string>())
      .description("Inlet velocity (vars x,y,z)")
      .attach_trigger ( boost::bind ( &SubsonicInFlowWeakBc::config_velocity_function, this ) )
      .mark_basic();

  velocity_function.variables("x,y,z");
}

////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::config_density_function()
{
  std::cout << FromHere().short_str() << std::endl;

  density_function.functions( options()["rho_in"].value< std::string >() );
  density_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::config_velocity_function()
{
  std::cout << FromHere().short_str() << std::endl;

  velocity_function.functions( options()["vel_in"].value< std::vector<std::string> >() );
  velocity_function.parse();
}

/////////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::execute()
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
