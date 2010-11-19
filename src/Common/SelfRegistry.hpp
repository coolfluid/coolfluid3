// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_SelfRegistry_hpp
#define CF_Common_SelfRegistry_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Common/SafePtr.hpp"
#include "Common/GeneralStorage.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  class ProviderBase;

//////////////////////////////////////////////////////////////////////////////

  /// This class is a singleton object which serves as registry for all the
  /// self-registration object providers.
  /// The only instance of this object is held by the ModuleRegister
  /// @see Common::ProviderBase
  /// @see Common::Provider
  /// @author Tiago Quintino
  class Common_API SelfRegistry : public boost::noncopyable
  {

  public:

    /// Constructor is private to allow only the friend classes to build it
    SelfRegistry();

    /// Default destructor is private to allow only the friend classes to destroy it
    ~SelfRegistry();

    /// Register a Object provider
    /// @param factory pointer to the provider
    void regist(ProviderBase* provider);

    /// Remove a registered provider
    /// @param name of the provider
    void unregist(const std::string& name, const std::string& type);

    /// Remove a registered provider
    /// @param name of the provider
    void unregist(ProviderBase* provider);

    /// Get a given provider by his name
    /// @param name of the provider
    /// @return a SafePtr to the provider
    SafePtr<ProviderBase> get_provider(const std::string& name, const std::string& type);

  private: // data

    std::map<std::string, GeneralStorage<ProviderBase> > m_store;

}; // SelfRegistry

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_SelfRegistry_hpp
