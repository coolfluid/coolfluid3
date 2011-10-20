// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/CRoot.hpp"
#include "common/CGroup.hpp"

#include "Mesh/Actions/LibActions.hpp"
#include "Mesh/Actions/LoadBalance.hpp"

namespace cf3 {
namespace Mesh {
namespace Actions {

using namespace cf3::common;

cf3::common::RegistLibrary<LibActions> libActions;

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
} // cf3
