// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"

#include "Mesh/Actions/LibActions.hpp"
#include "Mesh/Actions/LoadBalance.hpp"

namespace CF {
namespace Mesh {
namespace Actions {

using namespace CF::Common;

CF::Common::RegistLibrary<LibActions> libActions;

const char * balancer_name = "LoadBalancer";

////////////////////////////////////////////////////////////////////////////////

void LibActions::initiate_impl()
{
//  Core::instance().tools()
//      .create_component_ptr<LoadBalance>( balancer_name )
//      ->mark_basic();
}

void LibActions::terminate_impl()
{
//  Core::instance().tools()
//      .remove_component( balancer_name );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
