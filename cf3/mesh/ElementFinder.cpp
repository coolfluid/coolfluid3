// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/ElementFinder.hpp"
#include "mesh/Dictionary.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

//////////////////////////////////////////////////////////////////////////////

ElementFinder::ElementFinder(const std::string &name) : common::Component(name)
{
  options().add("dict",m_dict)
      .description("Dictionary used to find the element")
      .link_to(&m_dict);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
