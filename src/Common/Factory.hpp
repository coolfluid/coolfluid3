#ifndef CF_Common_Factory_hpp
#define CF_Common_Factory_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Log.hpp"
#include "Common/SafePtr.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/FactoryRegistry.hpp"
#include "Common/Provider.hpp"
#include "Common/CoreEnv.hpp"

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

    /// @return the name of the type of this factory
    virtual std::string getTypeName() const = 0;

    /// @return all the providers in this Factory in a std::vector
    virtual std::vector<Common::ProviderBase*> getAllProviders() const = 0;

    /// Get a given Provider
    /// @throw Common::ValueNotFound if the Provider is not registered
    virtual ProviderBase* getProviderBase (const std::string& name) const = 0;

  }; // end class FactoryBase

/// Stores the Provider's that create self-registration objects polymorphically.
/// It is part of an abstract factory pattern implementation.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class Factory : public FactoryBase {

public: // methods

  /// @return the instance of this singleton
  static Common::Factory<BASE>& getInstance();

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
  virtual std::string getTypeName() const { return BASE::getClassName(); }

  /// @return all the providers in this Factory
  virtual std::vector<Common::ProviderBase*> getAllProviders() const;

  /// Get a given Provider
  /// @throw Common::ValueNotFound if the Provider is not registered
  virtual Common::SafePtr< typename BASE::PROVIDER > getProvider(const std::string& name) const;

  /// Get a given Provider, cast as a ProviderBase
  /// @throw Common::ValueNotFound if the Provider is not registered
  virtual  ProviderBase* getProviderBase(const std::string& name) const;

protected: // helper functions

  /// Constructor is protected because this is a Singleton.
  Factory();
  /// Destructor is protected because this is a Singleton.
  ~Factory();

  /// providers database
  std::map<std::string, Common::Provider<BASE>*>& getProviderMap() { return m_providers; }

  /// providers database
  const std::map<std::string, Common::Provider<BASE>*>& getProviderMap() const { return m_providers; }

private: // data

  /// providers database
  std::map<std::string, Provider<BASE>*> m_providers;

}; // end of class Factory

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>& Factory<BASE>::getInstance()
{
  static Common::Factory<BASE> obj;
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>::Factory()
{
  Common::CoreEnv::getInstance().getFactoryRegistry()->regist(this);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Factory<BASE>::~Factory()
{
//  Common::CoreEnv::getInstance().getFactoryRegistry()->unregist( getTypeName() );
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
void Factory<BASE>::regist(Provider<BASE>* provider)
{
  if (exists(provider->getName()))
  {
    throw Common::ValueExists (FromHere(),
      "In factory of [" + BASE::getClassName() +
      "] a provider with the name [" + provider->getName() +
      "] was found when trying to regist it" );
  }
  getProviderMap()[provider->getName()] = provider;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
bool Factory<BASE>::exists(const std::string& name) const
{
  return (getProviderMap().count(name) > 0);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
void Factory<BASE>::unregist(const std::string& providerName)
{
  if (!exists(providerName))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + BASE::getClassName() +
      "] a provider with the name [" + providerName +
      "] was not found while trying to unregist it" );
  }
  getProviderMap().erase(getProviderMap().find(providerName));
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
ProviderBase* Factory<BASE>::getProviderBase(const std::string& name) const
{
  if (!exists(name))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + getTypeName() +
      "] a provider with the name [" + name +
      "] was not found while trying to get the provider" );
  }

  return getProviderMap().find(name)->second;
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
Common::SafePtr< typename BASE::PROVIDER >
Factory<BASE>::getProvider(const std::string& providerName) const
{
  if (!exists(providerName))
  {
    throw Common::ValueNotFound (FromHere(),
      "In factory of [" + BASE::getClassName() +
      "] a provider with the name [" + providerName +
      "] was not found while trying to get the provider" );
  }

  return dynamic_cast<typename BASE::PROVIDER*>
    (getProviderMap().find(providerName)->second);
}

////////////////////////////////////////////////////////////////////////////////

template <class BASE>
std::vector<Common::ProviderBase*> Factory<BASE>::getAllProviders() const
{
  std::vector<Common::ProviderBase*> result;
  typename std::map<std::string,Provider<BASE>*>::const_iterator itr;
  itr = getProviderMap().begin();
  for (; itr != getProviderMap().end(); ++itr) {
    result.push_back(itr->second);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Factory_hpp
