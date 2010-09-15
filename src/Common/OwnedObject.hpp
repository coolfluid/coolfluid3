// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OwnedObject_hpp
#define CF_Common_OwnedObject_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Base class for objects that will be held by SharedPtr or SelfRegistPtr
/// @see SharedPtr
/// @see SelfRegistPtr
/// @author Andrea Lani
/// @author Tiago Quintino

class Common_API OwnedObject {

public: // functions

  /// Default constructor
  OwnedObject() : m_owners(0) {}

  /// Virtual destructor
  virtual ~OwnedObject() {}

  /// Add an owner.
  void addOwner() { m_owners++; }

  /// Remove an owner.
  void removeOwner() { m_owners--; }

  /// Check if the object is not owned anymore.
  bool hasNoOwner() { return (m_owners == 0) ? true : false; }

  /// Get the number of owners
  CF::Uint getNbOwners() const { return m_owners; }

private: // data

  /// Counter for the number of owners of this object
  CF::Uint m_owners;

}; // end class OwnedObject

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OwnedObject_hpp
