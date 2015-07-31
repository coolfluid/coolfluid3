// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "common/Builder.hpp"
#include "common/Signal.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/TaggedComponentFilter.hpp"
#include "common/FindComponents.hpp"

#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TaggedComponentFilter, ComponentFilter, LibCommon > TaggedComponentFilter_Builder;

TaggedComponentFilter::TaggedComponentFilter ( const std::string& name ) : ComponentFilter(name)
{
  options().add("tag", m_tag)
    .pretty_name("Tag")
    .description("Tag value to filter on")
    .link_to(&m_tag)
    .mark_basic();
}

bool TaggedComponentFilter::operator()(const Component& component)
{
  return component.has_tag(m_tag);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
