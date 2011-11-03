// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/core/NGeneric.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

NGeneric::NGeneric(const std::string & name, const QString & type, CNode::Type node_type) :
    CNode(name, type, node_type)
{

}

////////////////////////////////////////////////////////////////////////////

QString NGeneric::tool_tip() const
{
  return this->component_type();
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
