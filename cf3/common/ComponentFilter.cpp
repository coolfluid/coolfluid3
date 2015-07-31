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
#include "common/ComponentFilter.hpp"
#include "common/FindComponents.hpp"

#include "common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

RegisterComponent<ComponentFilter,LibCommon> register_ComponentFilter;

ComponentFilter::ComponentFilter ( const std::string& name ) : Component(name)
{
}

bool ComponentFilter::operator()(const Handle<Component const>& component)
{
  if(is_null(component))
    return false;

  return this->operator()(*component);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
