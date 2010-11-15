// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionArray.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

OptionArray::OptionArray(const std::string& name,
                        const std::string& desc, const boost::any def) :
    Option(name, desc, def)
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
