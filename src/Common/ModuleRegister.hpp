// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ModuleRegister_hpp
#define CF_Common_ModuleRegister_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ModuleRegisterBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a module register template.
/// @author Tiago Quintino
template < typename MODULE >
class ModuleRegister : public Common::ModuleRegisterBase {
public: // methods

    /// Acessor to the singleton
  static MODULE& instance();

  /// Returns the description of the module.
  /// Must be implemented by the ModuleRegister
  /// @return descripton of the module
  virtual std::string getDescription() const;

protected: // methods

    /// Constructor
  ModuleRegister();

    /// Destructor
  ~ModuleRegister();

}; // end class ModuleRegister

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
MODULE& ModuleRegister<MODULE>::instance()
{
  static MODULE instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
ModuleRegister<MODULE>::ModuleRegister() :
ModuleRegisterBase(MODULE::getModuleName())
{
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
ModuleRegister<MODULE>::~ModuleRegister()
{
}

////////////////////////////////////////////////////////////////////////////////

template < typename MODULE >
std::string ModuleRegister<MODULE>::getDescription() const
{
  return MODULE::getModuleDescription();
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ModuleRegister_hpp
