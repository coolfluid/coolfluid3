// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LibraryRegister_hpp
#define CF_Common_LibraryRegister_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/LibraryRegisterBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a module register template.
/// @author Tiago Quintino
template < typename MODULE >
class LibraryRegister : public Common::LibraryRegisterBase {
public: // methods

    /// Acessor to the singleton
  static MODULE& instance();

  /// Returns the description of the module.
  /// Must be implemented by the LibraryRegister
  /// @return descripton of the module
  virtual std::string getDescription() const;

protected: // methods

    /// Constructor
  LibraryRegister();

    /// Destructor
  ~LibraryRegister();

}; // end class LibraryRegister

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
MODULE& LibraryRegister<MODULE>::instance()
{
  static MODULE instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
LibraryRegister<MODULE>::LibraryRegister() :
LibraryRegisterBase(MODULE::library_name())
{
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
LibraryRegister<MODULE>::~LibraryRegister()
{
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
std::string LibraryRegister<MODULE>::getDescription() const
{
  return MODULE::library_description();
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LibraryRegister_hpp
