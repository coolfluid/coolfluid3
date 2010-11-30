// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo can be removed when removing "count" option,
/// as well as the "using namespace boost::assign;" directive.
#include <boost/assign/std/vector.hpp>

#include "Common/CGroup.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"

using namespace boost::assign; // for operator+=()

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CGroup, Component, LibCommon > CGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

CGroup::CGroup ( const std::string& name ) : Component ( name )
{
    define_config_properties();
}

////////////////////////////////////////////////////////////////////////////////

CGroup::~CGroup()
{
}

////////////////////////////////////////////////////////////////////////////////

void CGroup::define_config_properties ()
{
  m_properties.add_option< OptionT<CF::Real> >("pi", "Pi in a CGroup", 3.141592);

  Option::Ptr opt = m_properties.add_option< OptionT<std::string> >("count", "From one to ten", std::string("One"));

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
