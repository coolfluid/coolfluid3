// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Core/NGeneric.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NGeneric::NGeneric(const QString & name, const QString & type) :
    CNode(name, type, GENERIC_NODE)
{

}

////////////////////////////////////////////////////////////////////////////

QString NGeneric::toolTip() const
{
  return this->getComponentType();
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // GUI
} // CF
