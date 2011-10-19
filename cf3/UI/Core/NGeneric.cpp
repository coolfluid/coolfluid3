// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NGeneric.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NGeneric::NGeneric(const std::string & name, const QString & type, CNode::Type nodeType) :
    CNode(name, type, nodeType)
{

}

////////////////////////////////////////////////////////////////////////////

QString NGeneric::toolTip() const
{
  return this->componentType();
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
