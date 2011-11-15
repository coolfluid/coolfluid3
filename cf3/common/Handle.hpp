// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Handle_hpp
#define cf3_common_Handle_hpp

#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/weak_ptr.hpp>

#include "common/Assertions.hpp"
#include "common/CommonAPI.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// Safe pointer to an object. This is the supported method for referring to components.
template<typename T>
class Common_API Handle
{
public:
  /// Default constructor, generating a null handle
  Handle() : m_cached_ptr(0) {}

  /// Construction from shared_ptr. This constructor may cast the argument:
  /// - If T is a base class of Y, a static cast is done
  /// - otherwise, a dynamic cast is done. If this fails, the resulting Handle is null.
  template<typename Y>
  explicit Handle(const boost::shared_ptr<Y>& ptr)
  {
    create_from_shared(ptr);
  }

  /// Construction from another handle. Casting is done as in construction from shared_ptr.
  template<typename Y>
  explicit Handle(const Handle<Y>& other)
  {
    create_from_shared(other.m_weak_ptr.lock());
  }
  
  /// Copy constructor also taking anon-const handle to create a const handle implicitely
  Handle(const Handle<typename boost::remove_const<T>::type>& other) :
    m_weak_ptr(other.m_weak_ptr),
    m_cached_ptr(other.m_cached_ptr)
  {
  }

  /// Raw pointer to the stored value, or null if there is none
  T* get() const
  {
    if(is_null(m_cached_ptr))
      return nullptr;

    if(m_weak_ptr.expired())
      m_cached_ptr = nullptr;

    return m_cached_ptr;
  }

  /// Set the handle to null
  void reset()
  {
    m_weak_ptr.reset();
    m_cached_ptr = 0;
  }

  /// Conversion to bool for null checking
  operator bool () const
  {
    return !m_weak_ptr.expired();
  }

  bool operator! () const
  {
    return m_weak_ptr.expired();
  }

  T* operator->() const
  {
    cf3_assert(!m_weak_ptr.expired());
    return this->get();
  }

  T& operator* () const // never throws
  {
    cf3_assert(!m_weak_ptr.expired());
    return *(this->get());
  }

  /// Internal compare function, modeled after boost::shared_ptr
  template<typename Y> bool _internal_less(Handle<Y> const & rhs) const
  {
    return m_weak_ptr < rhs.m_weak_ptr;
  }

private:
  /// Helper function for the constructors
  template<typename Y>
  void create_from_shared(const boost::shared_ptr<Y>& ptr)
  {
    create_from_shared(boost::is_base_of<T, Y>(), ptr);
  }

  /// Avoid the is_base_of stuff when both types are the same
  void create_from_shared(const boost::shared_ptr<T>& ptr)
  {
    m_weak_ptr = ptr;
    m_cached_ptr = ptr.get();
  }

  /// Static dispatch in case T is a base of Y
  template<typename Y>
  void create_from_shared(const boost::true_type, const boost::shared_ptr<Y>& ptr)
  {
    m_weak_ptr = boost::static_pointer_cast<T>(ptr);
    m_cached_ptr = m_weak_ptr.lock().get();
  }

  /// Static dispatch in case T is not a base of Y
  template<typename Y>
  void create_from_shared(const boost::false_type, const boost::shared_ptr<Y>& ptr)
  {
    m_weak_ptr = boost::dynamic_pointer_cast<T>(ptr);
    m_cached_ptr = m_weak_ptr.lock().get();
  }

  /// Weak pointer to the original shared pointer for the component
  boost::weak_ptr<T> m_weak_ptr;

  /// Cached pointer after the first dereference
  mutable T* m_cached_ptr;

  /// Needed for private access from the casting constructors
  template<class Y> friend class Handle;
};

template<typename T, typename U> inline bool operator==(const Handle<T>& a, const Handle<U>& b)
{
  return a.get() == b.get();
}

template<typename T, typename U> inline bool operator==(const Handle<T>& a, const U b)
{
  return a.get() == b;
}

template<typename T, typename U> inline bool operator==(const U a, const Handle<T>& b)
{
  return a == b.get();
}

template<typename T, typename U> inline bool operator!=(const Handle<T>& a, const Handle<U>& b)
{
  return a.get() != b.get();
}

template<typename T, typename U> inline bool operator!=(const Handle<T>& a, const U b)
{
  return a.get() != b;
}

template<typename T, typename U> inline bool operator!=(const U a, const Handle<T>& b)
{
  return a != b.get();
}

template<typename T, typename U> inline bool operator<(const Handle<T>& a, const Handle<U>& b)
{
  return a._internal_less(b);
}

/// Helper function to make a handle
template<typename T>
Handle<T> make_handle(const boost::shared_ptr<T>& p)
{
  return Handle<T>(p);
}

} // common
} // cf3

#endif // cf3_common_Handle_hpp
