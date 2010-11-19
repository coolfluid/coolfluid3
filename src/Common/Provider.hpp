// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Provider_hpp
#define CF_Common_Provider_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/NamedObject.hpp"
#include "Common/ProviderBase.hpp"
#include "Common/Factory.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

  template < class BASE > class Factory;

////////////////////////////////////////////////////////////////////////////////

/// @brief Templated class provider
/// Used in the template abstract factory with Self Registering Objects.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class Provider : public Common::NamedObject,
                 public Common::ProviderBase
{
public: // methods

  /// Constructor
  /// @param name provider registration name
  Provider(const std::string& name) :
    Common::NamedObject(name),
    Common::ProviderBase()
  {
    Common::Factory<BASE>::instance().regist(this);
  }

  /// Virtual destructor
  virtual ~Provider() {}

  /// @return the name of this provider
  virtual std::string provider_name () const { return getName(); }

  /// @return the BASE of this provider
  virtual std::string provider_type () const { return BASE::type_name(); }

}; // Provider

////////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

////////////////////////////////////////////////////////////////////////////////
#endif // CF_Common_Provider_hpp
