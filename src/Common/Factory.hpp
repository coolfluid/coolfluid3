// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Factory_hpp
#define CF_Common_Factory_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>

#include "Common/Log.hpp"
#include "Common/SafePtr.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/Provider.hpp"
#include "Common/Core.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  template < class BASE > class Provider;

////////////////////////////////////////////////////////////////////////////////


  /// This class serves as a base class for factory's so that all factories
  /// can be held by a FactoryRegistry
  /// @author Tiago Quintino
  class Common_API FactoryBase : public boost::noncopyable {

  public: // methods

    /// destructor
    virtual ~FactoryBase() {}

    /// @return the name of the type of this factory
    virtual std::string factory_type_name() const = 0;

    /// @return all the providers in this Factory in a std::vector
    virtual std::vector<Common::ProviderBase*> get_all_providers() const = 0;

    /// Get a given Provider
    /// @throw Common::ValueNotFound if the Provider is not registered
    virtual ProviderBase* get_provider_base (const std::string& name) const = 0;

  }; // end class FactoryBase

/// Stores the Provider's that create self-registration objects polymorphically.
/// It is part of an abstract factory pattern implementation.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class Factory : public FactoryBase {

public: // methods

  /// @return the instance of this singleton
  static Common::Factory<BASE>& instance();

  /// Checks if a provider is registered
  /// @param name name of the provider
  bool exists(const std::string& name) const;

  /// Registers a provider
  /// @param provider pointer to the provider to be registered
  void regist(Common::Provider<BASE>* provider);

  /// Remove a registered provider
  /// @throw Common::ValueNotFound if the provider is not registered
  void unregist(const std::string& providerName);

  /// @return the name of the BASE of this factory
  virtual std::string factory_type_name() const { return BASE::type_name(); }

  /// @return all the providers in this Factory
  virtual std::vector<Common::ProviderBase*> get_all_providers() const;

  virtual std::vector<typename BASE::PROVIDER* > get_all_concrete_providers() const;

  /// Get a given Provider
  /// @throw Common::ValueNotFound if the Provider is not registered
  virtual Common::SafePtr< typename BASE::PROVIDER > get_provider(const std::string& name) const;

  /// Get a given Provider, cast as a ProviderBase
  /// @throw Common::ValueNotFound if the Provider is not registered
  virtual  ProviderBase* get_provider_base(const std::string& name) const;

protected: // helper functions

  /// Constructor is protected because this is a Singleton.
  Factory();
  /// Destructor is protected because this is a Singleton.
  virtual ~Factory();

  /// providers database
  std::map<std::string, Common::Provider<BASE>*>& provider_map() { return m_providers; }

  /// providers database
  const std::map<std::string, Common::Provider<BASE>*>& provider_map() const { return m_providers; }

private: // data

  /// providers database
  std::map<std::string, Provider<BASE>*> m_providers;

}; // Factory

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>& Factory<BASE>::instance()
{
  static Common::Factory<BASE> obj;
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>::Factory()
{
  Common::Core::instance().factory_registry().lock()->regist(this);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>::~Factory()
{
//  Common::CoreEnv::instance().factory_registry()->unregist( factory_type_name() );
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
void Factory<BASE>::regist(Provider<BASE>* provider)
{
  if (exists(provider->name()))
  {
    throw Common::ValueExists (FromHere(),
      "In factory of [" + BASE::type_name() +
      "] a provider with the name [" + provider->name() +
      "] was found when trying to regist it" );
  }
  provider_map()[provider->name()] = provider;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
bool Factory<BASE>::exists(const std::string& name) const
{
  return (provider_map().count(name) > 0);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
void Factory<BASE>::unregist(const std::string& providerName)
{
  if (!exists(providerName))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + BASE::type_name() +
      "] a provider with the name [" + providerName +
      "] was not found while trying to unregist it" );
  }
  provider_map().erase(provider_map().find(providerName));
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
ProviderBase* Factory<BASE>::get_provider_base(const std::string& name) const
{
  if (!exists(name))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + factory_type_name() +
      "] a provider with the name [" + name +
      "] was not found while trying to get the provider" );
  }

  return provider_map().find(name)->second;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Common::SafePtr< typename BASE::PROVIDER >
Factory<BASE>::get_provider(const std::string& providerName) const
{
  if (!exists(providerName))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + BASE::type_name() +
      "] a provider with the name [" + providerName +
      "] was not found while trying to get the provider" );
  }

  return dynamic_cast<typename BASE::PROVIDER*>
    (provider_map().find(providerName)->second);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
std::vector<Common::ProviderBase*> Factory<BASE>::get_all_providers() const
{
  std::vector<Common::ProviderBase*> result;
  typename std::map<std::string,Provider<BASE>*>::const_iterator itr;
  itr = provider_map().begin();
  for (; itr != provider_map().end(); ++itr) {
    result.push_back(itr->second);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
std::vector<typename BASE::PROVIDER*> Factory<BASE>::get_all_concrete_providers() const
{
  std::vector<typename BASE::PROVIDER*> result;
  typename std::map<std::string,Provider<BASE>*>::const_iterator itr;
  itr = provider_map().begin();
  for (; itr != provider_map().end(); ++itr) {
    result.push_back(dynamic_cast<typename BASE::PROVIDER*>(itr->second));
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Factory_hpp
