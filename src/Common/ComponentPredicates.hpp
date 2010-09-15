// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ComponentPredicates_hpp
#define CF_Common_ComponentPredicates_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/filter_iterator.hpp>
#include <boost/foreach.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>

#include "Common/ComponentIterator.hpp"
#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////
// Filter iterator classes and functions
////////////////////////////////////////////////////////////////////////////////

// Some pre-prepared Predicates

class IsComponentTrue
{
public:

  template<typename T>
  bool operator()(const T& component)
  { return true; }

};

class IsComponentName
{
private:
  std::string m_name;
public:
  IsComponentName () : m_name("undefined") {}
  IsComponentName (const std::string& name) : m_name(name) {}

  bool operator()(const Component::Ptr& component)
  { return boost::bind( std::equal_to<std::string>() , boost::bind(&Component::name , _1) , m_name )(component); }

  bool operator()(const Component& component)
  { return boost::bind( std::equal_to<std::string>() , boost::bind(&Component::name , _1) , m_name )(component); }

};

class IsComponentTag
{
private:
  std::string m_tag;
public:
  IsComponentTag () : m_tag("Component") {}
  IsComponentTag (const std::string& tag) : m_tag(tag) {}

  bool operator()(const Component::Ptr& component)
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

  bool operator()(const Component& component)
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

};

template<class CType, class Predicate=IsComponentTrue>
class IsComponentType
{
private:
  std::string m_type;
  Predicate m_pred;
public:
  IsComponentType (Predicate pred) : m_type(CType::type_name()), m_pred(pred) {}
  IsComponentType () : m_type(CType::type_name()), m_pred() {}

  bool operator()(const Component::Ptr& component)
  { return boost::bind( &Component::has_tag , _1 , m_type )(component) && m_pred(component); }

  bool operator()(const Component& component)
  { return boost::bind( &Component::has_tag , _1 , m_type )(component) && m_pred(component); }

};

////////////////////////////////////////////////////////////////////////////////
// Range type shorthands and automatic const deduction
////////////////////////////////////////////////////////////////////////////////

/// Shorthand for a range of two filtered iterators
template<typename IteratorT, typename Predicate=IsComponentTrue>
struct FilteredIteratorRange {
  typedef boost::iterator_range<boost::filter_iterator<Predicate, IteratorT> > type;

  /// Returns a range filtered by the given iterator type
  inline static type make_range(const IteratorT& from, const IteratorT& to , const Predicate& pred)
  {
    return boost::make_iterator_range(boost::filter_iterator<Predicate, IteratorT>(pred,from,to),
                                      boost::filter_iterator<Predicate, IteratorT>(pred,to,to));
  }
};

/// Specialization without the predicate and no filter
template<typename IteratorT>
struct FilteredIteratorRange<IteratorT, IsComponentTrue> {
  typedef boost::iterator_range<IteratorT> type;

  /// Returns a bare, unfiltered range
  inline static type make_range(const IteratorT& from, const IteratorT& to, const IsComponentTrue&)
  {
    return boost::make_iterator_range(from, to);
  }
};

/// Derive the correct range type based on the constness of ParentT, which should be the type of the parent component
template<typename ParentT, typename ComponentT=Component, typename Predicate=IsComponentTrue>
struct ComponentIteratorRange {
  typedef typename FilteredIteratorRange<typename boost::mpl::if_c<boost::is_const<ParentT>::value, // if ParentT is const
                                                                    ComponentIterator<ComponentT const>, // use a const component iterator
                                                                    ComponentIterator<ComponentT> >::type, // or the mutable one otherwise
                                          Predicate>::type type;
};

/// Reference to ComponentT, constness determined by the constness of ParentT
template<typename ParentT, typename ComponentT=Component>
struct ComponentReference {
  typedef typename boost::mpl::if_c<boost::is_const<ParentT>::value, ComponentT const&, ComponentT&>::type type;
};

/// Shared pointer to ComponentT, constness determined by the constness of ParentT
template<typename ParentT, typename ComponentT=Component>
struct ComponentPtr {
  typedef typename boost::mpl::if_c<boost::is_const<ParentT>::value, boost::shared_ptr<ComponentT const>, boost::shared_ptr<ComponentT> >::type type;
};

////////////////////////////////////////////////////////////////////////////////
// Wrappers to make iterating easy
////////////////////////////////////////////////////////////////////////////////

/// Given two iterators delimiting a component range, return a range that conforms to the
/// given predicate. The unfiltered version is not provided, since boost::make_iterator_range(from, to) is equivalent.
template <typename Predicate, typename IteratorT>
inline typename FilteredIteratorRange<IteratorT, Predicate>::type
make_filtered_range(const IteratorT& from, const IteratorT& to , const Predicate& pred)
{
  return FilteredIteratorRange<IteratorT, Predicate>::make_range(from, to, pred);
}

/// Filtered range of all direct children of type ComponentT
/// Note: ParentT is a template argument instead of base class Component to allow const and non-const versions in one go
template <typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type
filtered_range_typed(ParentT& parent, const Predicate& pred)
{
  return make_filtered_range(parent.template begin<ComponentT>(),parent.template end<ComponentT>(),pred);
}

/// Filtered range of all direct children
template <typename ParentT, typename Predicate>
inline typename ComponentIteratorRange<ParentT, Component, Predicate>::type
filtered_range(ParentT& parent, const Predicate& pred)
{
  return filtered_range_typed<Component>(parent, pred);
}

/// Creates a range containing all components of ComponentT from the direct children
template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRange<ParentT, ComponentT>::type
range_typed(ParentT& parent)
{
  return boost::make_iterator_range(parent.template begin<ComponentT>(),parent.template end<ComponentT>());
}

/// Range of all direct children
template <typename ParentT>
inline typename ComponentIteratorRange<ParentT, Component>::type
range(ParentT& parent)
{
  return range_typed<Component>(parent);
}

/// Creates a range containing all components of type ComponentT from the whole tree under parent, filtered by the predicate
template <typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type
recursive_filtered_range_typed(ParentT& parent, const Predicate& pred)
{
  return make_filtered_range(parent.template recursive_begin<ComponentT>(),parent.template recursive_end<ComponentT>(),pred);
}

/// Filtered range of all subcomponentes
template <typename ParentT, typename Predicate>
inline typename ComponentIteratorRange<ParentT, Component, Predicate>::type
recursive_filtered_range(ParentT& parent, const Predicate& pred)
{
  return recursive_filtered_range_typed<Component>(parent, pred);
}

/// Creates a range containing all components of ComponentT from the whole tree under parent
template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRange<ParentT, ComponentT>::type
recursive_range_typed(ParentT& parent)
{
  return boost::make_iterator_range(parent.template recursive_begin<ComponentT>(),parent.template recursive_end<ComponentT>());
}

/// Range of all subcomponents, recursive
template <typename ParentT>
inline typename ComponentIteratorRange<ParentT, Component>::type
recursive_range(ParentT& parent)
{
  return recursive_range_typed<Component>(parent);
}

////////////////////////////////////////////////////////////////////////////////
// Component querying functions
////////////////////////////////////////////////////////////////////////////////

/// Gets a reference to the unique component of the given type that satisfies the given predicate.
/// Throws if there is not exactly one match
template<typename ComponentT, typename ParentT, typename Predicate>
typename ComponentReference<ParentT, ComponentT>::type
get_component_typed(ParentT& parent, const Predicate& pred )
{
  typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type range = filtered_range_typed<ComponentT>(parent, pred);
  if(range.begin() == range.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.full_path().string() + " : 0 matches");
  else if(count(range) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.full_path().string() + " : more than 1 match");
  else
    return *range.begin();
}

/// Gets a reference to the unique component of the given type that satisfies the given predicate.
/// Throws if there is not exactly one match
template<typename ComponentT, typename ParentT >
typename ComponentReference<ParentT, ComponentT>::type
get_component_typed(ParentT& parent )
{
  return get_component_typed<ComponentT> ( parent, IsComponentTrue() );
}

/// Gets a reference to the unique component that satisfies the given predicate.
/// Throws if there is not exactly one match
template<typename ParentT, typename Predicate>
inline typename ComponentReference<ParentT>::type
get_component(ParentT& parent, const Predicate& pred)
{
  return get_component_typed<Component>(parent, pred);
}

/// Gets a shared pointer to the unique component of the given type that satisfies the given predicate.
/// Result is null if there is no match or if there is more than 1 match
template<typename ComponentT, typename ParentT, typename Predicate>
typename ComponentPtr<ParentT, ComponentT>::type
get_component_typed_ptr(ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type range = filtered_range_typed<ComponentT>(parent, pred);
  typedef typename ComponentPtr<ParentT, ComponentT>::type ResultT;
  if(range.begin() == range.end() || count(range) > 1)
    return ResultT();
  return range.begin().base().get();
}

/// Gets a shared pointer to the unique component of the given type that satisfies the given predicate.
/// Result is null if there is no match or if there is more than 1 match
template<typename ParentT, typename Predicate>
inline typename ComponentPtr<ParentT>::type
get_component_ptr(ParentT& parent, const Predicate& pred)
{
  return get_component_typed_ptr<Component>(parent, pred);
}

/// Reference to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
get_named_component_typed(ParentT& parent, const Component::CName& name) {
  return get_component_typed<ComponentT>(parent, IsComponentName(name));
}

/// Reference to the component with the given name
template<typename ParentT>
inline typename ComponentReference<ParentT>::type
get_named_component(ParentT& parent, const Component::CName& name) {
  return get_named_component_typed<Component>(parent, name);
}

/// Pointer to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentPtr<ParentT, ComponentT>::type
get_named_component_typed_ptr(ParentT& parent, const Component::CName& name) {
  return get_component_typed_ptr<ComponentT>(parent, IsComponentName(name));
}

/// Pointer to the component with the given name
template<typename ParentT>
inline typename ComponentPtr<ParentT>::type
get_named_component_ptr(ParentT& parent, const Component::CName& name) {
  return get_named_component_typed_ptr<Component>(parent, name);
}
  
/// Reference to the component with the given tag
template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
get_tagged_component_typed(ParentT& parent, const std::string& tag) {
  return get_component_typed<ComponentT>(parent, IsComponentTag(tag));
}

/// Reference to the component with the given tag
template<typename ParentT>
inline typename ComponentReference<ParentT>::type
get_tagged_component(ParentT& parent, const std::string& tag) {
  return get_tagged_component_typed<Component>(parent, tag);
}

/// Pointer to the component with the given tag
template<typename ComponentT, typename ParentT>
inline typename ComponentPtr<ParentT, ComponentT>::type
get_tagged_component_typed_ptr(ParentT& parent, const std::string& tag) {
  return get_component_typed_ptr<ComponentT>(parent, IsComponentTag(tag));
}

/// Pointer to the component with the given tag
template<typename ParentT>
inline typename ComponentPtr<ParentT>::type
get_tagged_component_ptr(ParentT& parent, const std::string& tag) {
  return get_tagged_component_typed_ptr<Component>(parent, tag);
}
  

/// Gets a reference to the unique component of the given type that satisfies the given predicate.
/// Throws if there is not exactly one match
template<typename ComponentT, typename ParentT, typename Predicate>
typename ComponentReference<ParentT, ComponentT>::type
recursive_get_component_typed(ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type range = recursive_filtered_range_typed<ComponentT>(parent, pred);
  if(range.begin() == range.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.full_path().string() + " : 0 matches");
  else if(count(range) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.full_path().string() + " : More than 1 match");
  else
    return *range.begin();
}

/// Gets a reference to the unique component that satisfies the given predicate.
/// Throws if there is not exactly one match
template<typename ParentT, typename Predicate>
inline typename ComponentReference<ParentT>::type
recursive_get_component(ParentT& parent, const Predicate& pred)
{
  return recursive_get_component_typed<Component>(parent, pred);
}

/// Gets a shared pointer to the unique component of the given type that satisfies the given predicate.
/// Result is null if there is no match or if there is more than 1 match
template<typename ComponentT, typename ParentT, typename Predicate>
typename ComponentPtr<ParentT, ComponentT>::type
recursive_get_component_typed_ptr(ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRange<ParentT, ComponentT, Predicate>::type range = recursive_filtered_range_typed<ComponentT>(parent, pred);
  typedef typename ComponentPtr<ParentT, ComponentT>::type ResultT;
  if(range.begin() == range.end() || count(range) > 1)
    return ResultT();
  return range.begin().base().get();
}

/// Gets a shared pointer to the unique component of the given type that satisfies the given predicate.
/// Result is null if there is no match or if there is more than 1 match
template<typename ParentT, typename Predicate>
inline typename ComponentPtr<ParentT>::type
recursive_get_component_ptr(ParentT& parent, const Predicate& pred)
{
  recursive_get_component_typed_ptr<Component>(parent, pred);
}

/// Reference to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
recursive_get_named_component_typed(ParentT& parent, const std::string& name) {
  return recursive_get_component_typed<ComponentT>(parent, IsComponentName(name));
}

/// Reference to the component with the given name
template<typename ParentT>
inline typename ComponentReference<ParentT>::type
recursive_get_named_component(ParentT& parent, const std::string& name) {
  return recursive_get_named_component_typed<Component>(parent, name);
}

/// Pointer to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentPtr<ParentT, ComponentT>::type
recursive_get_named_component_typed_ptr(ParentT& parent, const std::string& name) {
  return get_component_typed_ptr<ComponentT>(parent, IsComponentName(name));
}

/// Pointer to the component with the given name
template<typename ParentT>
inline typename ComponentPtr<ParentT>::type
recursive_get_named_component_ptr(ParentT& parent, const std::string& name) {
  return get_named_component_typed_ptr<Component>(parent, name);
}
  
/// Reference to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
recursive_get_tagged_component_typed(ParentT& parent, const std::string& tag) {
  return recursive_get_component_typed<ComponentT>(parent, IsComponentName(tag));
}

/// Reference to the component with the given name
template<typename ParentT>
inline typename ComponentReference<ParentT>::type
recursive_get_tagged_component(ParentT& parent, const std::string& tag) {
  return recursive_get_tagged_component_typed<Component>(parent, tag);
}

/// Pointer to the component with the given name
template<typename ComponentT, typename ParentT>
inline typename ComponentPtr<ParentT, ComponentT>::type
recursive_get_tagged_component_typed_ptr(ParentT& parent, const std::string& tag) {
  return get_component_typed_ptr<ComponentT>(parent, IsComponentName(tag));
}

/// Pointer to the component with the given name
template<typename ParentT>
inline typename ComponentPtr<ParentT>::type
recursive_get_tagged_component_ptr(ParentT& parent, const std::string& tag) {
  return get_tagged_component_typed_ptr<Component>(parent, tag);
}

////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////

/// Count the elements in a range
template<typename RangeT>
Uint count(const RangeT& range) {
  Uint result = 0;
  for(typename RangeT::const_iterator it = range.begin(); it != range.end(); ++it)
    ++result;
  return result;
}

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentPredicates_hpp
