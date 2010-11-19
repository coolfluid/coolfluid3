// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo can be removed when removing "count" option,
/// as well as the "using namespace boost::assign;" directive.
#include <boost/assign/std/vector.hpp>

#include "Common/CGroup.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"

using namespace boost::assign; // for operator+=()

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CGroup, Component, LibCommon, NB_ARGS_1 >
CGroup_Provider ( CGroup::type_name() );

////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const CName& name ) : Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////

void CGroup::define_config_properties ( Common::PropertyList& options )
{
  options.add_option< OptionT<CF::Real> >("pi", "Pi in a CGroup", 3.141592);

  Option::Ptr opt = options.add_option< OptionT<std::string> >("count", "From one to ten", std::string("One"));

  opt->restricted_list() += std::string("Two"),
                            std::string("Three"),
                            std::string("Four"),
                            std::string("Five"),
                            std::string("Six"),
                            std::string("Seven"),
                            std::string("Eight"),
                            std::string("Nine"),
                            std::string("Ten");

  opt->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
