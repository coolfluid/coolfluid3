// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_ElementCaching_hpp
#define cf3_SFDM_ElementCaching_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>

#include "common/Log.hpp"
#include "common/StringConversion.hpp"

#include "mesh/Reconstructions.hpp"
#include "mesh/Entities.hpp"

#include "SFDM/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

/// ElementCache base class is a component. It is counted on not many of these objects will be created
/// using a caching/locking mechanism
class ElementCacheBase //: public common::Component
{
public:

  static std::string type_name() { return "ElementCacheBase"; }
  ElementCacheBase(const std::string& name) //: common::Component(name)
  {
    idx = math::Consts::uint_max();
    CFdebug <<"Create new " << name;
    unlock();
  }
  virtual ~ElementCacheBase() {}

  virtual void configure(const Handle<const mesh::Entities>& entities_comp)
  {
    CFdebug<<" for " << entities_comp->uri() <<CFendl;
    // reset and reconfigure for this element type
    unlock();
    if(entities)
    {
      if (entities_comp==entities)
      {
        return;
      }
    }
    // compute if it was not configured yet
    idx = math::Consts::uint_max();
    entities = entities_comp;
    compute_fixed_data();
  }

  // mark this object as locked, must unlock manualy
  void compute_element(const Uint elem_idx)
  {
    lock();
    if (idx != elem_idx)
    {
      idx = elem_idx;
      compute_variable_data();
    }
  }

  Handle< mesh::Entities const      > entities;
  Uint idx;

  // locking mechanism
  bool locked() const { return m_locked; }
  void lock() { m_locked=true; }
  void unlock() { m_locked=false; }

private:

  virtual void compute_fixed_data() = 0;
  virtual void compute_variable_data() = 0;

private:
  bool m_locked;
};

////////////////////////////////////////////////////////////////////////////////

template <typename ElementCache>
class Pool : public common::Component
{
public:
  typedef ElementCache element_type;

  typedef mesh::Entities const* key_type;
  typedef boost::shared_ptr<element_type> data_type;
  typedef std::map<key_type,data_type> value_type;
  Pool(const std::string& name) :
    common::Component(name)
  {
  }
  static std::string type_name() { return "Pool"; }

  virtual void configure_cache(ElementCache& cache, const Handle<mesh::Entities const>& entities )
  {
    cache.configure(entities);
  }

  virtual void set_cache(ElementCache& cache, const Uint elem)
  {
    cache.compute_element(elem);
  }

  /// destructor
  virtual ~Pool()
  {
    while(!m_element_caches.empty())
    {
      typename value_type::iterator it = m_element_caches.begin();
      m_element_caches.erase(it);
    }
  }

  /// Create ElementCache if non-existant, else get the ElementCache and lock it through ElementCacheHandle constructor
  Handle<ElementCache> cache(const Handle<mesh::Entities const>& entities)
  {
    typename value_type::iterator it = m_element_caches.find(entities.get());
    if(it != m_element_caches.end())
    {
      if (it->second->locked()) throw common::IllegalCall(FromHere(),"cache is locked to elem "+common::to_str(it->second->idx));
      return make_handle(it->second);
    }
    boost::shared_ptr<ElementCache> element_cache ( new ElementCache );
    configure_cache(*element_cache,entities);
    m_element_caches[entities.get()]=element_cache;
    return make_handle(element_cache);
  }

  /// Create ElementCache if non-existant, else get the ElementCache and lock it through ElementCacheHandle constructor
  Handle<ElementCache> cache(const Handle<mesh::Entities const>& entities, const Uint elem)
  {
    typename value_type::iterator it = m_element_caches.find(entities.get());
    if(it != m_element_caches.end())
    {
      if ( it->second->idx == elem ) // Nothing to be done
      {
        it->second->lock();
        return make_handle(it->second);
      }

      if ( it->second->locked() )
        throw common::IllegalCall(FromHere(),"cache is locked to elem "+common::to_str(it->second->idx));

      set_cache(*it->second,elem);
      return make_handle(it->second);
    }
    boost::shared_ptr<ElementCache> element_cache ( new ElementCache );
    configure_cache(*element_cache,entities);
    set_cache(*element_cache,elem);
    m_element_caches[entities.get()]=element_cache;
    return make_handle(element_cache);
  }

private:
  value_type m_element_caches;
};


////////////////////////////////////////////////////////////////////////////////

class PoolFactory : public common::Component
{
public:
  static std::string type_name() { return "PoolFactory"; }
  PoolFactory(const std::string& name) : common::Component(name) {}
  virtual ~PoolFactory() {}


  template <typename ElementCache>
  Handle< typename ElementCache::CachePool > pool(const std::string& key = ElementCache::type_name())
  {
    ++factory_calls;
    Handle< typename ElementCache::CachePool > fac = Handle< typename ElementCache::CachePool >(get_child(key));
    if (!fac) // if not available, generate it
    {
      CFdebug << "Creating Pool for " << key << CFendl;
      fac = create_component< typename ElementCache::CachePool >(key);
    }
    return fac;
  }
  static Uint factory_calls;
};
Uint PoolFactory::factory_calls = 0;

////////////////////////////////////////////////////////////////////////////////

struct DummyElementCache : ElementCacheBase
{
  typedef Pool<DummyElementCache> CachePool;
  static std::string type_name() { return "DummyElementCache"; }
  DummyElementCache (const std::string& name=type_name()) : ElementCacheBase(name) {}
private:
  virtual void compute_fixed_data() {}
  virtual void compute_variable_data() {}
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_ElementCaching_hpp
