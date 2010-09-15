// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_FactoryRegistry_hpp
#define CF_Common_FactoryRegistry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/SafePtr.hpp"
#include "Common/GeneralStorage.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class FactoryBase;

////////////////////////////////////////////////////////////////////////////////

/// Registry for all the factory objects that are created.
/// Typically this object is held by the CoreEnv singleton,
/// therefore only one of these registries exists.
/// @see Factory
/// @author Tiago Quintino
class Common_API FactoryRegistry : public boost::noncopyable {

public:

  /// Register a factory
  /// @param module pointer to a FactoryBase to be added
  void regist(Common::FactoryBase* factory);

  /// Remove a registered factory
  /// @param name name of a FactoryBase to be removed
  void unregist(const std::string& name);

  /// Get a given factory
  /// @param name name of a FactoryBase to be accessed
  /// @return a pointer to a FactoryBase if found or a null pointer if not found
  Common::SafePtr<Common::FactoryBase> getFactory(const std::string& name);

private: // data

  Common::GeneralStorage<Common::FactoryBase> m_store;

}; // FactoryRegistry

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_FactoryRegistry_hpp
