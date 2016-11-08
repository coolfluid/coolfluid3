// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_DereferenceComponent_hpp
#define cf3_common_DereferenceComponent_hpp

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

namespace detail
{
  template<typename ComponentT>
  struct Dereference
  {
    ComponentT& operator()(ComponentT& comp)
    {
      return comp;
    }
  };

  template<template<typename> class SmartPointerT, typename ComponentT>
  struct Dereference<SmartPointerT<ComponentT>>
  {
    ComponentT& operator()(const SmartPointerT<ComponentT>& comp)
    {
      return *comp;
    }
  };

  template<typename ComponentT>
  struct Dereference<ComponentT*>
  {
    ComponentT& operator()(ComponentT* comp)
    {
      return *comp;
    }
  };
}

/// Generic method to dereference a pointer, handle or reference to a component
template<typename RefT>
inline auto dereference(RefT& referred_component) -> decltype(detail::Dereference<RefT>()(referred_component))
{
  return detail::Dereference<RefT>()(referred_component);
}

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_DereferenceComponent_hpp
