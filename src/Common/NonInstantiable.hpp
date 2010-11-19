// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_NonInstantiable_hpp
#define CF_Common_NonInstantiable_hpp

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Derive from this class if you want a class that is not instantiable.
template < class TYPE >
class NonInstantiable {
private:
    NonInstantiable ();
};

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_NonInstantiable_hpp
