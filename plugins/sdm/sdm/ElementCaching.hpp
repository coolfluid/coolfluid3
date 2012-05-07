// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ElementCaching_hpp
#define cf3_sdm_ElementCaching_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/OptionList.hpp"
#include "common/StringConversion.hpp"

#include "math/Consts.hpp"

#include "mesh/Entities.hpp"

#include "sdm/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

class Cache : public common::Component
{
public:
  Cache(const std::string& name) : common::Component(name), options_added(false) { }
  static std::string type_name() { return "Cache"; }
  virtual ~Cache() {}

public:
  bool options_added;
};

////////////////////////////////////////////////////////////////////////////////

/// ElementCache base class is a component. It is counted on not many of these objects will be created
/// using a caching/locking mechanism
class ElementCache //: public common::Component
{
public:

  static std::string type_name() { return "ElementCache"; }
  ElementCache(const std::string& name) //: common::Component(name)
  {
    idx = math::Consts::uint_max();
    unlock();
  }
  virtual ~ElementCache() {}

  virtual void configure(const Handle<const mesh::Entities>& entities_comp)
  {
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

  /// This object contains all user-configurable options
  Handle< Cache const> cache;

  const common::OptionList& options() const;

  // locking mechanism
  bool locked() const { return m_locked; }
  void lock() { m_locked=true; }
  void unlock() { m_locked=false; }

  static void add_options(Cache& cache) { throw common::NotImplemented(FromHere(),"Type of object "+cache.uri().string()+" must override add_options()"); }
private:

  virtual void compute_fixed_data() = 0;
  virtual void compute_variable_data() = 0;

private:
  bool m_locked;
};


////////////////////////////////////////////////////////////////////////////////

inline const common::OptionList& ElementCache::options() const
{
  return cache->options();
}

////////////////////////////////////////////////////////////////////////////////

template <typename ElementCacheT>
class CacheT : public Cache
{
public:
  typedef ElementCacheT element_type;

  typedef mesh::Entities const* key_type;
  typedef boost::shared_ptr<element_type> data_type;
  typedef std::map<key_type,data_type> value_type;
  CacheT(const std::string& name) :
    Cache(name),
    m_cache(nullptr)
  { }
  static std::string type_name() { return "Cache"; }

  void add_options()
  {
    if (options_added == false)
    {
      ElementCacheT::add_options( *this );
      options_added = true;
    }
  }

  virtual ElementCacheT& configure_cache( const Handle<mesh::Entities const>& entities )
  {
    get().configure(entities);
    return get();
  }

  virtual ElementCacheT& set_cache(const Uint elem)
  {
    get().compute_element(elem);
    return get();
  }

  /// destructor
  virtual ~CacheT()
  {
    while(!m_element_caches.empty())
    {
      typename value_type::iterator it = m_element_caches.begin();
      m_element_caches.erase(it);
    }
  }

  /// Create ElementCache if non-existant, else get the ElementCache and lock it through ElementCacheHandle constructor
  ElementCacheT& cache(const Handle<mesh::Entities const>& entities)
  {
    typename value_type::iterator it = m_element_caches.find(entities.get());
    if(it != m_element_caches.end())
    {
      m_cache = it->second.get();

      if (get().locked()) throw common::IllegalCall(FromHere(),"cache "+uri().string()+" is locked to elem "+common::to_str(it->second->idx));

      return get();
    }
    boost::shared_ptr<ElementCacheT> element_cache ( new ElementCacheT );
    m_cache = element_cache.get();
    m_cache->cache = this->handle<Cache>();
    configure_cache(entities);
    m_element_caches[entities.get()]=element_cache;
    return get();
  }

  /// Create ElementCache if non-existant, else get the ElementCache and lock it through ElementCacheHandle constructor
  ElementCacheT& cache(const Handle<mesh::Entities const>& entities, const Uint elem)
  {
    typename value_type::iterator it = m_element_caches.find(entities.get());
    if(it != m_element_caches.end())
    {
      m_cache=it->second.get();

      if ( get().idx == elem ) // Nothing to be done
      {
        get().lock();
        return get();
      }

      if (get().locked()) throw common::IllegalCall(FromHere(),"cache "+uri().string()+" is locked to elem "+common::to_str(it->second->idx));

      set_cache(elem);
      return get();
    }
    boost::shared_ptr<ElementCacheT> element_cache ( new ElementCacheT );
    m_cache=element_cache.get();
    m_cache->cache = this->handle<Cache>();
    configure_cache(entities);
    set_cache(elem);
    m_element_caches[entities.get()]=element_cache;
    return get();
  }

  ElementCacheT& get()
  {
    cf3_assert(is_not_null(m_cache));
    return *m_cache;
  }
  ElementCacheT* m_cache;

private:

  value_type m_element_caches;
};


////////////////////////////////////////////////////////////////////////////////

class SharedCaches : public common::Component
{
public:
  static std::string type_name() { return "SharedCaches"; }
  SharedCaches(const std::string& name) : common::Component(name) {}
  virtual ~SharedCaches() {}

  /// Get or create a shared cache object
  /// @note this function should be avoided in loops, as it uses internally
  /// the somewhat expensive function Component::get_child()
  template <typename ElementCacheT>
  Handle< typename ElementCacheT::cache_type > get_cache(const std::string& tag = ElementCacheT::type_name())
  {
    Handle< typename ElementCacheT::cache_type > fac = Handle< typename ElementCacheT::cache_type >(get_child(tag));
    if (!fac) // if not available, generate it
    {
      fac = create_component< typename ElementCacheT::cache_type >(tag);
      fac->add_options();
    }
    return fac;
  }

  void reset_shared_caches()
  {
    throw common::NotImplemented(FromHere(),"Create a non-templated ElementCache base function with function to reset, and call here on every child");
  }
};

////////////////////////////////////////////////////////////////////////////////

struct DummyElementCache : ElementCache
{
  typedef CacheT<DummyElementCache> cache_type;
  static std::string type_name() { return "DummyElementCache"; }
  DummyElementCache (const std::string& name=type_name()) : ElementCache(name) {}
private:
  static void add_options(Cache& cache) {}
  virtual void compute_fixed_data() {}
  virtual void compute_variable_data() {}
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_ElementCaching_hpp
