// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_ComponentRange_hpp
#define cf3_common_ComponentRange_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/ptr_container/ptr_vector.hpp>

#include "common/Handle.hpp"

namespace cf3 {
namespace common {

class Component;

////////////////////////////////////////////////////////////////////////////////

/// Iterator for ranges
template <class ComponentT, class RangeT>
class RangeIterator
  : public boost::iterator_facade<
                                   RangeIterator<ComponentT, RangeT>
                                   , ComponentT
                                   , boost::forward_traversal_tag
                                  >
{
public:

  explicit RangeIterator(const RangeT& range, const typename RangeT::PositionT& position)
    : m_range(range), m_position(position)
  {
  }

  template <class OtherValue>
  RangeIterator(RangeIterator<OtherValue, RangeT> const& other)
    : m_range(other.m_range), m_position(other.m_position)
  {
  }

private:
  friend class boost::iterator_core_access;
  template <class,class> friend class RangeIterator;

  /// We assume this is only called on iterators from the same range
  template <class OtherValue>
  bool equal(RangeIterator<OtherValue, RangeT> const& other) const
  {
    return this->m_position == other.m_position;
  }

  void increment()
  {
    m_range.increment(m_position);
  }

  ComponentT& dereference() const
  {
    return m_range.dereference(m_position);
  }

  typename RangeT::PositionT m_position;
  const RangeT& m_range;
};

/// Traits class to make the component type available to RangeBase
template<typename RangeT>
struct RangeTraits;

template<typename RangeT>
using component_type = typename RangeTraits<RangeT>::ComponentT;

template<typename BaseRangeT, typename PredicateT>
class FilteredComponentRange;

/// Base class (CRTP) for ranges
template<typename DerivedT>
class RangeBase
{
public:
  typedef RangeIterator<component_type<DerivedT>, DerivedT> iterator;
  typedef RangeIterator<component_type<DerivedT> const, DerivedT> const_iterator;

  /// Filter the range using the given predicate
  template<typename PredicateT>
  FilteredComponentRange<DerivedT, PredicateT> filter(PredicateT predicate);

  /// C++ iterator interface
  /// Begin iterator
  iterator begin()
  {
    return iterator(derived(), derived().start_position());
  }
  const_iterator begin() const
  {
    return const_iterator(derived(), derived().start_position());
  }

  /// End iterator
  iterator end()
  {
    return iterator(derived(), derived().end_position());
  }
  const_iterator end() const
  {
    return const_iterator(derived(), derived().end_position());
  }

  /// CRTP implementation
  const DerivedT& derived() const
  {
    return *static_cast<const DerivedT*>(this);
  }
};

/// @brief Safely keep a random-access list that refers to components without taking ownership.
///
/// Used to hold safe references (handles) to a number of components, loosely based on the boost ptr_vector paradigm but not taking ownership of the stored items
template<typename ComponentTPar>
class Common_API ComponentVector : public RangeBase<ComponentVector<ComponentTPar>>
{
public:
  /// Represent a postition in the range
  typedef Uint PositionT;
  typedef ComponentTPar ComponentT;

  /// Construct the range, optionally adding the goven number of items
  explicit ComponentVector ( const Uint nb_entries = 0 ) : m_components(nb_entries)
  {
  }

  /// Allocate space without resizing
  void reserve(Uint size)
  {
    m_components.reserve(size);
  }

  /// Resize
  void resize(const Uint size)
  {
    m_components.resize(size);
  }

  /// Return the number of held elements
  Uint size() const
  {
    return m_components.size();
  }

  /// Clear
  void clear()
  {
    m_components.clear();
  }

  /// Append an element using a reference
  void push_back(ComponentT& component)
  {
    push_back(component.template handle<ComponentT>());
  }

  /// Append an element using a handle
  void push_back(const Handle<ComponentT>& component)
  {
    m_components.push_back(component);
  }

  /// Element access
  ComponentT& operator[](const Uint idx) const
  {
    cf3_assert(is_not_null(handle(idx)));
    return *handle(idx);
  }

  /// Check for null
  bool is_null(const Uint idx)
  {
    return handle(idx).get() == 0;
  }

  /// Direct access to the handle
  Handle<ComponentT> handle(const Uint idx) const
  {
    cf3_assert(idx < m_components.size());
    return m_components[idx];
  }

  /// Range interface
  /// Increment the given position, taking into account types
  void increment(PositionT& position) const
  {
    cf3_assert(position != m_components.size());
    ++position;
  }

  /// Return the component at the given postition
  ComponentT& dereference(const PositionT& position) const
  {
    cf3_assert(position < m_components.size());
    return *m_components[position];
  }

  PositionT start_position() const
  {
    return 0;
  }

  PositionT end_position() const
  {
    return m_components.size();
  }

private:

  typedef std::vector< Handle<ComponentT> > StorageT;
  StorageT m_components;
}; // ComponentVector

template<typename ComponentTPar>
struct RangeTraits<ComponentVector<ComponentTPar>>
{
  typedef ComponentTPar ComponentT;
};

/// Lazy component range, strongly typed on ComponentT
template<typename ComponentTPar>
class Common_API BasicComponentRange : public RangeBase<BasicComponentRange<ComponentTPar>>
{
  typedef std::vector<boost::shared_ptr<Component>> StorageT;

public:
  /// Represent a postition in the range
  typedef Uint PositionT;
  typedef ComponentTPar ComponentT;

  template<typename ParentT>
  explicit BasicComponentRange(const ParentT& parent) : m_components(parent.m_components)
  {
  }

  /// Coolfluid range interface
  /// Increment the given position, taking into account types
  void increment(PositionT& position) const
  {
    cf3_assert(position != m_components.size()); // attempt to increment end
    ++position;
    while(position != m_components.size() && (dynamic_cast<ComponentT*>(m_components[position].get()) == nullptr))
    {
      ++position;
    }
  }

  /// Return the component at the given postition
  ComponentT& dereference(const PositionT& position) const
  {
    cf3_assert(position < m_components.size());
    ComponentT* result = dynamic_cast<ComponentT*>(m_components[position].get());
    cf3_assert(result != nullptr);
    return *result;
  }

  /// Position of the first valid element or 0 if there is none
  PositionT start_position() const
  {
    if(m_components.empty())
      return 0;

    PositionT result = 0;
    if(dynamic_cast<ComponentT*>(m_components[result].get()) == nullptr)
      increment(result);
    return result;
  }

  /// Position corresponding to the end iterator
  PositionT end_position() const
  {
    return m_components.size();
  }

private:
  const StorageT& m_components;
};

template<typename ComponentTPar>
struct RangeTraits<BasicComponentRange<ComponentTPar>>
{
  typedef ComponentTPar ComponentT;
};

/// Filtered component range
template<typename BaseRangeT, typename PredicateT>
class Common_API FilteredComponentRange : public RangeBase<FilteredComponentRange<BaseRangeT, PredicateT>>
{
public:
  /// Represent a postition in the range
  typedef typename BaseRangeT::PositionT PositionT;
  typedef typename BaseRangeT::ComponentT ComponentT;

  explicit FilteredComponentRange(const BaseRangeT& base_range, PredicateT predicate) : m_base_range(base_range), m_predicate(predicate)
  {
  }

  /// Range interface
  /// Increment the given position, taking into account types
  void increment(PositionT& position) const
  {
    const PositionT end_position = this->end_position();
    cf3_assert(position != end_position); // attempt to increment end
    m_base_range.increment(position);
    while(position != end_position && !m_predicate(m_base_range.dereference(position)))
    {
      m_base_range.increment(position);
    }
  }

  /// Return the component at the given postition
  ComponentT& dereference(const PositionT& position) const
  {
    return m_base_range.dereference(position);
  }

  /// Position of the first valid element or 0 if there is none
  PositionT start_position() const
  {
    if(m_base_range.start_position() == m_base_range.end_position())
      return m_base_range.start_position();

    PositionT result = m_base_range.start_position();
    if(!m_predicate(m_base_range.dereference(result)))
      increment(result);
    return result;
  }

  /// Position corresponding to the end iterator
  PositionT end_position() const
  {
    return m_base_range.end_position();
  }

  const BaseRangeT& m_base_range;
  PredicateT m_predicate;
};

template<typename BaseRangeT, typename PredicateT>
struct RangeTraits<FilteredComponentRange<BaseRangeT, PredicateT>>
{
  typedef typename RangeTraits<BaseRangeT>::ComponentT ComponentT;
};

template<typename ComponentT=common::Component, typename ParentT>
BasicComponentRange<ComponentT> make_range(ParentT& parent)
{
  return BasicComponentRange<ComponentT>(parent);
}

template<class DerivedT> template<class PredicateT>
FilteredComponentRange<DerivedT, PredicateT> RangeBase<DerivedT>::filter(PredicateT predicate)
{
  return FilteredComponentRange<DerivedT, PredicateT>(derived(), predicate);
}


////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_ComponentRange_hpp
