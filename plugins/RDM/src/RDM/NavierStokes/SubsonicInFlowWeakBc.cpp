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

#include "RDM/NavierStokes/SubsonicInFlowWeakBc.hpp"

#include "Physics/NavierStokes/Cons2D.hpp"


using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SubsonicInFlowWeakBc, RDM::BoundaryTerm, LibRDM > SubsonicInFlowWeakBc_Builder;

Common::ComponentBuilder < FaceLoopT< SubsonicInFlowWeakBc, Physics::NavierStokes::Cons2D>, RDM::FaceLoop, LibRDM > SubsonicInFlowWeakBc_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SubsonicInFlowWeakBc::SubsonicInFlowWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  m_options.add_option< OptionT<std::string> > ("rho_in", std::string() )
      ->description("Inlet density (vars x,y,z)")
      ->attach_trigger ( boost::bind ( &SubsonicInFlowWeakBc::config_density_function, this ) )
      ->mark_basic();

  density_function.variables("x,y,z");

  m_options.add_option< OptionArrayT<std::string> > ("vel_in",std::vector<std::string>())
      ->description("Inlet velocity (vars x,y,z)")
      ->attach_trigger ( boost::bind ( &SubsonicInFlowWeakBc::config_velocity_function, this ) )
      ->mark_basic();

  velocity_function.variables("x,y,z");
}

////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::config_density_function()
{
  std::cout << FromHere().short_str() << std::endl;

  density_function.functions( m_options["rho_in"].value< std::string >() );
  density_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::config_velocity_function()
{
  std::cout << FromHere().short_str() << std::endl;

  velocity_function.functions( m_options["vel_in"].value< std::vector<std::string> >() );
  velocity_function.parse();
}

/////////////////////////////////////////////////////////////////////////////////////

void SubsonicInFlowWeakBc::execute()
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
