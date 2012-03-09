// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_FindComponents_hpp
#define cf3_common_FindComponents_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/ComponentIterator.hpp"
#include "common/Foreach.hpp"

namespace cf3 {
namespace common {

/// Prevent a crash in the intel compiler
struct StringConverter
{
  StringConverter(const std::string& str) : m_value(str.c_str())
  {
  }
  
  StringConverter(const char* c_str) : m_value(c_str)
  {
  }
  
  std::string str()
  {
    return std::string(m_value);
  }
  
  operator char const*()
  {
    return m_value;
  }
  
  const char* m_value;
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

/// Handle to ComponentT, constness determined by the constness of ParentT
template<typename ParentT, typename ComponentT=Component>
struct ComponentHandle {
  typedef typename boost::mpl::if_c<boost::is_const<ParentT>::value, Handle<ComponentT const>, Handle<ComponentT> >::type type;
};

/// Derive the correct range type based on the constness of ParentT, which should be the type of the parent component
template<typename ParentT, typename ComponentT=Component>
struct ComponentIteratorSelector {
  typedef typename boost::mpl::if_c<boost::is_const<ParentT>::value, // if ParentT is const
                                    ComponentIterator<ComponentT const>,  // use a const component iterator
                                    ComponentIterator<ComponentT> >::type // or the mutable one otherwise
          type;
};

template<typename ComponentT, typename ParentT>
inline typename ComponentIteratorSelector<ParentT,ComponentT>::type component_begin(ParentT& component)
{
  std::vector< typename ComponentPtr<ParentT,ComponentT>::type > vec;
  component.template put_components<ComponentT>(vec, false); // not recursive
  return typename ComponentIteratorSelector<ParentT,ComponentT>::type(vec, 0); // begin
}

template<typename ComponentT, typename ParentT>
inline typename ComponentIteratorSelector<ParentT,ComponentT>::type component_end(ParentT& component)
{
  std::vector< typename ComponentPtr<ParentT,ComponentT>::type > vec;
  component.template put_components<ComponentT>(vec, false);  // not recursive
  return typename ComponentIteratorSelector<ParentT,ComponentT>::type(vec, vec.size()); // end
}

template<typename ComponentT, typename ParentT>
inline typename ComponentIteratorSelector<ParentT,ComponentT>::type component_recursive_begin(ParentT& component)
{
  std::vector< typename ComponentPtr<ParentT,ComponentT>::type > vec;
  component.template put_components<ComponentT>(vec, true); // recursive
  return typename ComponentIteratorSelector<ParentT,ComponentT>::type(vec, 0); // begin
}

template<typename ComponentT, typename ParentT>
inline typename ComponentIteratorSelector<ParentT,ComponentT>::type component_recursive_end(ParentT& component)
{
  std::vector< typename ComponentPtr<ParentT,ComponentT>::type > vec;
  component.template put_components<ComponentT>(vec, true); // recursive
  return typename ComponentIteratorSelector<ParentT,ComponentT>::type(vec, vec.size()); // end
}

////////////////////////////////////////////////////////////////////////////////
// Filter iterator classes and functions
////////////////////////////////////////////////////////////////////////////////

// Some pre-prepared Predicates

class IsComponentTrue
{
public:

  template<typename T>
  bool operator()(const T& component) const
  { return true; }

};

class IsComponentName
{
private:
  std::string m_name;
public:
  IsComponentName () : m_name("undefined") {}
  IsComponentName (StringConverter name) : m_name(name) {}

  bool operator()(const Handle<Component const>& component) const
  { return boost::bind( std::equal_to<std::string>() , boost::bind(&Component::name , _1) , m_name )(component.get()); }

  bool operator()(const Component& component) const
  { return boost::bind( std::equal_to<std::string>() , boost::bind(&Component::name , _1) , m_name )(component); }

};

class IsComponentTag
{
private:
  std::string m_tag;
public:
  IsComponentTag () : m_tag("Component") {}
  IsComponentTag (StringConverter tag) : m_tag(tag) {}

  bool operator()(const Handle<Component const>& component) const
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component.get()); }

  bool operator()(const Component& component) const
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

};

template<class CType>
class IsComponentType
{
public:
  IsComponentType () {}

  bool operator()(const Handle<Component const>& component) const
  { return operator()(*component); }

  bool operator()(const Component& component) const
  { return is_not_null( dynamic_cast<CType const*>(&component) ); }

};

////////////////////////////////////////////////////////////////////////////////
// Range type shorthands and automatic const deduction
////////////////////////////////////////////////////////////////////////////////

template<typename T, typename Predicate=IsComponentTrue>
struct ComponentIteratorRange : //public iterator_range<typename T::const_iterator>::type
                                       public boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T> > >
{
  typedef boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T> > > Base;

  typedef boost::filter_iterator< Predicate,  ComponentIterator<T> > iterator;


  typedef ComponentIteratorRange type;

  ComponentIteratorRange ( const ComponentIterator<T>& b, const ComponentIterator<T>& e )
  : Base ( iterator( Predicate() , b , e ) ,
           iterator( Predicate() , e , e ) )
  {}

  ComponentIteratorRange ( const ComponentIterator<T>& b, const ComponentIterator<T>& e , const Predicate& pred )
  : Base ( iterator( pred , b , e ) ,
           iterator( pred , e , e ) )
  {}

  ComponentIteratorRange ( const std::vector< boost::shared_ptr<T> >& vec )
    : Base ( iterator( Predicate() , ComponentIterator<T>(vec,0),          ComponentIterator<T>(vec,vec.size()) ) ,
             iterator( Predicate() , ComponentIterator<T>(vec,vec.size()), ComponentIterator<T>(vec,vec.size()) ) )
  {}

  bool operator==( const ComponentIteratorRange& rhs )  { return equal( rhs) ; }
  bool operator!=( const ComponentIteratorRange& rhs )  { return !equal( rhs) ; }

  std::vector< Handle<T> > as_vector()
  {
    std::vector< Handle<T> > result (0);
    BOOST_FOREACH ( T& val, *this )
      result.push_back(val.template handle<T>());
    return result;
  }

  std::vector< Handle<T const> > as_const_vector()
  {
    std::vector< Handle<T const> > result (0);
    BOOST_FOREACH ( const T& val, *this )
      result.push_back(val.template handle<T>());
    return result;
  }

  Uint size() const
  {
    Uint result = 0;
    iterator it = this->begin();
    while ( it != this->end() )
    {
      ++result;
      ++it;
    }
    return result;
  }
};


template<typename T, typename Predicate=IsComponentTrue>
struct ConstComponentIteratorRange : //public iterator_range<typename T::const_iterator>::type
                                       public boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T const> > >
{
  typedef boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T const> > > Base;
  typedef boost::filter_iterator< Predicate, ComponentIterator<T const> > ConstFilter;
  typedef boost::filter_iterator< Predicate, ComponentIterator<T> >       Filter;

  ConstComponentIteratorRange ( ComponentIterator<T const> b, ComponentIterator<T const> e )
  : Base ( ConstFilter( Predicate() , b , e ) ,
           ConstFilter( Predicate() , e , e ) )
  {}

  ConstComponentIteratorRange ( ComponentIterator<T const> b, ComponentIterator<T const> e , const Predicate& pred )
  : Base ( ConstFilter( pred , b , e ) ,
           ConstFilter( pred , e , e ) )
  {}

  ConstComponentIteratorRange( const ComponentIteratorRange<T,Predicate>& rhs  )
  : Base ( rhs.begin(), rhs.end() )
  {}

  ConstComponentIteratorRange ( const std::vector< boost::shared_ptr<T> >& vec )
    : Base ( ConstFilter( Predicate() , vec.begin() , vec.end() ) ,
             ConstFilter( Predicate() , vec.end()   , vec.end() ) )
  {}

  bool operator==( const ConstComponentIteratorRange& rhs )  { return equal( rhs) ; }
  bool operator!=( const ConstComponentIteratorRange& rhs )  { return !equal( rhs) ; }

  std::vector<boost::shared_ptr<T const> > as_vector()
  {
    std::vector<boost::shared_ptr<T> > result (0);
    BOOST_FOREACH ( const T& val, *this )
      result.push_back(val.template as_ptr<T>());
    return result;
  }

  std::vector<boost::shared_ptr<T const> > as_const_vector()
  {
    return as_vector();
  }

  Uint size() const
  {
    Uint result = 0;
    ConstFilter it = this->begin();
    while ( it != this->end() )
    {
      ++result;
      ++it;
    }
    return result;
  }
};



template <typename T, typename Predicate>
inline ComponentIteratorRange<T, Predicate>
make_new_range(ComponentIterator<T> from, ComponentIterator<T> to , const Predicate& pred = IsComponentTrue() )
{
  return ComponentIteratorRange<T,Predicate>(from,to,pred);
}


// template<typename T, typename Predicate=IsComponentTrue>
// struct ConstComponentIteratorRange : //public iterator_range<typename T::const_iterator>::type
// public boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T const> > >
// {
// //  typedef ComponentIterator<T> iterator;
// //  typedef ComponentIterator<T const> const_iterator;
//   typedef boost::iterator_range< boost::filter_iterator< Predicate,  ComponentIterator<T const> > > Base;
//
//   template <typename T2>
//   ConstComponentIteratorRange ( const T2& c )
//   : Base ( c.template begin<T>(), c.template end<T>() )
//   {}
//
//   ConstComponentIteratorRange (  ComponentIterator<T> b, ComponentIterator<T> e )
//   : Base ( b, e )
//   {}
//
//   ConstComponentIteratorRange ( ComponentIterator<T const> b, ComponentIterator<T const> e )
//   : Base ( b, e )
//   {}
//
//   ConstComponentIteratorRange( const ComponentIteratorRange<T>& rhs  )
//   : Base ( rhs.begin(), rhs.end() )
//   {}
//
//   // ConstComponentIteratorRange( const ConstComponentIteratorRange<T>& rhs  )
//   // : Base ( rhs.begin(), rhs.end() )
//   // {}
//
//   bool operator==( const ConstComponentIteratorRange& rhs )  { return equal( rhs) ; }
//   bool operator!=( const ConstComponentIteratorRange& rhs )  { return !equal( rhs) ; }
//
// };

/// Derive the correct range type based on the constness of ParentT, which should be the type of the parent component
template<typename ParentT, typename ComponentT=Component, typename Predicate=IsComponentTrue>
struct ComponentIteratorRangeSelector {

  template<typename IsAbstractT, int dummy = 0>
  struct impl;

  template<int dummy>
  struct impl<boost::true_type, dummy>
  {
    typedef typename ComponentIteratorRange<typename boost::mpl::if_c<boost::is_const<ParentT>::value, // if ParentT is const
                                                                     ComponentT const, // use a const component iterator
                                                                     ComponentT >::type, // or the mutable one otherwise
                                           IsComponentTrue>::type type;
  };

  template<int dummy>
  struct impl<boost::false_type, dummy>
  {
    typedef typename ComponentIteratorRange<typename boost::mpl::if_c<boost::is_const<ParentT>::value, // if ParentT is const
                                                                     ComponentT const, // use a const component iterator
                                                                     ComponentT >::type, // or the mutable one otherwise
                                           Predicate>::type type;
  };

  typedef typename impl< typename boost::is_abstract<Predicate>::type >::type type;
};


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

////////////////////////////////////////////////////////////////////////////////

/// Create a vector of handles of components, given an iterator_range
template <typename T>
inline std::vector< Handle<T> > range_to_vector( boost::iterator_range<ComponentIterator<T> > range)
{
  std::vector< Handle<T> > result (0);
  BOOST_FOREACH ( T& val, range)
    result.push_back(val.template handle<T>());
  return result;
}

template <typename T, typename Predicate>
inline std::vector< Handle<T> > range_to_vector( boost::iterator_range<boost::filter_iterator<Predicate, ComponentIterator<T> > > range)
{
  std::vector< Handle<T> > result (0);
  BOOST_FOREACH ( T& val, range)
    result.push_back(val.template handle<T>());
  return result;
}

template <typename T>
inline std::vector< Handle<const T> > range_to_const_vector( boost::iterator_range<ComponentIterator<T> > range)
{
  std::vector< Handle<const T> > result (0);
  BOOST_FOREACH ( T& val, range)
    result.push_back(val.template handle<T>());
  return result;
}

template <typename T, typename Predicate>
inline std::vector< Handle<const T> > range_to_const_vector( boost::iterator_range<boost::filter_iterator<Predicate, ComponentIterator<T> > > range)
{
  std::vector< Handle<const T> > result (0);
  BOOST_FOREACH ( T& val, range)
    result.push_back(val.template handle<T const>());
  return result;
}


////////////////////////////////////////////////////////////////////////////////
// Wrappers to make iterating easy
////////////////////////////////////////////////////////////////////////////////

/// Given two iterators delimiting a component range, return a range that conforms to the
/// given predicate. The unfiltered version is not provided, since boost::make_iterator_range(from, to) is equivalent.
template <typename Predicate, typename T>
inline typename ComponentIteratorRange<T,Predicate>::type
make_filtered_range(const ComponentIterator<T>& from, const ComponentIterator<T>& to , const Predicate& pred)
{
  return ComponentIteratorRange<T,Predicate>(from,to,pred);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline typename ComponentIteratorRange<T,IsComponentTrue>::type
make_filtered_range(const ComponentIterator<T>& from, const ComponentIterator<T>& to)
{
  return ComponentIteratorRange<T,IsComponentTrue>(from,to,IsComponentTrue());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component>::type
find_components(Component& parent)
{
  return make_filtered_range(parent.begin(),parent.end());
  // return boost::make_iterator_range(component_begin(parent),component_end(parent));
}

inline ComponentIteratorRangeSelector<const Component, Component>::type
find_components(const Component& parent)
{
  return make_filtered_range(parent.begin(),parent.end());
}

template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type
find_components(ParentT& parent)
{
  return make_filtered_range(component_begin<ComponentT>(parent),component_end<ComponentT>(parent));
}

//////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type
find_components_with_filter(Component& parent, const Predicate& pred)
{
  return make_filtered_range(parent.begin(),parent.end(),pred);
}

template <typename Predicate>
inline typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type
find_components_with_filter(const Component& parent, const Predicate& pred)
{
  return make_filtered_range(parent.begin(),parent.end(),pred);
}


template <typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentIteratorRangeSelector<ParentT, ComponentT, Predicate>::type
find_components_with_filter(ParentT& parent, const Predicate& pred)
{
  return make_filtered_range(component_begin<ComponentT>(parent),component_end<ComponentT>(parent),pred);
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component, IsComponentName>::type
find_components_with_name(Component& parent, StringConverter name)
{
  return make_filtered_range(parent.begin(),parent.end(),IsComponentName(name));
}

inline ComponentIteratorRangeSelector<Component const, Component, IsComponentName>::type
find_components_with_name(const Component& parent, StringConverter name)
{
  return make_filtered_range(parent.begin(),parent.end(),IsComponentName(name));
}

template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRangeSelector<ParentT, ComponentT, IsComponentName>::type
find_components_with_name(ParentT& parent, StringConverter name)
{
  return make_filtered_range(component_begin<ComponentT>(parent),component_end<ComponentT>(parent),IsComponentName(name));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component, IsComponentTag>::type
find_components_with_tag(Component& parent, StringConverter tag)
{
  return make_filtered_range(parent.begin(),parent.end(),IsComponentTag(tag));
}

inline ComponentIteratorRangeSelector<Component const, Component, IsComponentTag>::type
find_components_with_tag(const Component& parent, StringConverter tag)
{
  return make_filtered_range(parent.begin(),parent.end(),IsComponentTag(tag));
}

template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRangeSelector<ParentT, ComponentT, IsComponentTag>::type
find_components_with_tag(ParentT& parent, StringConverter tag)
{
  return make_filtered_range(component_begin<ComponentT>(parent),component_end<ComponentT>(parent),IsComponentTag(tag));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component>::type
find_components_recursively(Component& parent)
{
  return make_filtered_range(parent.recursive_begin(),parent.recursive_end());
  // return boost::make_iterator_range(component_recursive_begin(parent),component_recursive_end(parent));
}

inline ComponentIteratorRangeSelector<Component const, Component>::type
find_components_recursively(const Component& parent)
{
  return make_filtered_range(parent.recursive_begin(),parent.recursive_end());
}

template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type
find_components_recursively(ParentT& parent)
{
  return make_filtered_range(component_recursive_begin<ComponentT>(parent),component_recursive_end<ComponentT>(parent));
}

//////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type
find_components_recursively_with_filter(Component& parent, const Predicate& pred)
{
  return make_filtered_range(parent.recursive_begin(),parent.recursive_end(),pred);
}

template <typename Predicate>
inline typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type
find_components_recursively_with_filter(const Component& parent, const Predicate& pred)
{
  return make_filtered_range(parent.recursive_begin(),parent.recursive_end(),pred);
}

template <typename ComponentT, typename Predicate>
inline typename ComponentIteratorRangeSelector<Component, ComponentT, Predicate>::type
find_components_recursively_with_filter(Component& parent, const Predicate& pred)
{
  return make_filtered_range(component_recursive_begin<ComponentT>(parent),component_recursive_end<ComponentT>(parent),pred);
}

template <typename ComponentT, typename Predicate>
inline typename ComponentIteratorRangeSelector<Component const, ComponentT, Predicate>::type
find_components_recursively_with_filter(const Component& parent, const Predicate& pred)
{
  return make_filtered_range(component_recursive_begin<ComponentT>(parent),component_recursive_end<ComponentT>(parent),pred);
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component, IsComponentName>::type
find_components_recursively_with_name(Component& parent, StringConverter name)
{
  return find_components_recursively_with_filter(parent,IsComponentName(name));
}

inline ComponentIteratorRangeSelector<Component const, Component, IsComponentName>::type
find_components_recursively_with_name(const Component& parent, StringConverter name)
{
  return find_components_recursively_with_filter(parent,IsComponentName(name));
}

template <typename ComponentT>
inline typename ComponentIteratorRangeSelector<Component, ComponentT, IsComponentName>::type
find_components_recursively_with_name(Component& parent, StringConverter name)
{
  return find_components_recursively_with_filter<ComponentT>(parent,IsComponentName(name));
}

template <typename ComponentT>
inline typename ComponentIteratorRangeSelector<Component const, ComponentT, IsComponentName>::type
find_components_recursively_with_name(const Component& parent, StringConverter name)
{
  return find_components_recursively_with_filter<ComponentT>(parent,IsComponentName(name));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentIteratorRangeSelector<Component, Component, IsComponentTag>::type
find_components_recursively_with_tag(Component& parent, StringConverter tag)
{
  return find_components_recursively_with_filter(parent,IsComponentTag(tag));
}

inline ComponentIteratorRangeSelector<Component const, Component, IsComponentTag>::type
find_components_recursively_with_tag(const Component& parent, StringConverter tag)
{
  return find_components_recursively_with_filter(parent,IsComponentTag(tag));
}

template <typename ComponentT>
inline typename ComponentIteratorRangeSelector<Component, ComponentT, IsComponentTag>::type
find_components_recursively_with_tag(Component& parent, StringConverter tag)
{
  return find_components_recursively_with_filter<ComponentT>(parent,IsComponentTag(tag));
}

template <typename ComponentT>
inline typename ComponentIteratorRangeSelector<Component const, ComponentT, IsComponentTag>::type
find_components_recursively_with_tag(const Component& parent, StringConverter tag)
{
  return find_components_recursively_with_filter<ComponentT>(parent,IsComponentTag(tag));
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component (Component& parent)
{
  ComponentIteratorRangeSelector<Component>::type r = find_components(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().path() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().path() + " : more than 1 match");
  else
    return *r.begin();
}

inline ComponentReference<Component const>::type
find_component (const Component& parent)
{
  ComponentIteratorRangeSelector<Component const>::type r = find_components(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().path() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().path() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename ComponentT, typename ParentT >
inline typename ComponentReference<ParentT, ComponentT>::type
find_component (ParentT& parent)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type r = find_components<ComponentT>(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component with type " + ComponentT::type_name() + " not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component with type " + ComponentT::type_name() + " not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

inline ComponentHandle<Component>::type
find_component_ptr (Component& parent)
{
  ComponentIteratorRangeSelector<Component>::type r = find_components(parent);
  typedef ComponentHandle<Component>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

inline ComponentHandle<Component const>::type
find_component_ptr (const Component& parent)
{
  ComponentIteratorRangeSelector<Component const>::type r = find_components(parent);
  typedef ComponentHandle<Component const>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr (ParentT& parent)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type r = find_components<ComponentT>(parent);
  typedef typename ComponentHandle<ParentT, ComponentT>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

//////////////////////////////////////////////////////////////////////////////

template<typename Predicate>
inline ComponentReference<Component>::type
find_component_with_filter (Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type r = find_components_with_filter(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Unique component with filter not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Unique component with filter not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename Predicate>
inline ComponentReference<Component const>::type
find_component_with_filter (const Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type r = find_components_with_filter(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Unique component with filter not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Unique component with filter not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_with_filter (ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT, Predicate>::type r = find_components_with_filter<ComponentT>(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Unique component with filter and type " + ComponentT::type_name() + " not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Unique component with filter and type " + ComponentT::type_name() + " not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename Predicate>
inline ComponentHandle<Component>::type
find_component_ptr_with_filter (Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type r = find_components_with_filter(parent, pred);
  typedef ComponentHandle<Component>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename Predicate>
inline ComponentHandle<Component const>::type
find_component_ptr_with_filter (const Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type r = find_components_with_filter(parent, pred);
  typedef ComponentHandle<Component const>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_with_filter (ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT, Predicate>::type r = find_components_with_filter<ComponentT>(parent, pred);
  typedef typename ComponentHandle<ParentT, ComponentT>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component_with_name (Component& parent, StringConverter name)
{
  try
  {
    return find_component_with_filter(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with name \""+name.str()+"\" not found in " + parent.uri().string());
  }
}

inline ComponentReference<Component const>::type
find_component_with_name (const Component& parent, StringConverter name) {
  try
  {
    return find_component_with_filter(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with name \""+name.str()+"\" not found in " + parent.uri().string());
  }
}
template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_with_name (ParentT& parent, StringConverter name) {
  try
  {
    return find_component_with_filter<ComponentT>(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with name \""+name.str()+"\" and type " + ComponentT::type_name() + " not found in " + parent.uri().string());
  }
}

inline ComponentHandle<Component>::type
find_component_ptr_with_name (Component& parent, StringConverter name)
{
  return find_component_ptr_with_filter(parent,IsComponentName(name));
}

inline ComponentHandle<Component const>::type
find_component_ptr_with_name (const Component& parent, StringConverter name)
{
  return find_component_ptr_with_filter(parent,IsComponentName(name));
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_with_name (ParentT& parent, StringConverter name)
{
  return find_component_ptr_with_filter<ComponentT>(parent,IsComponentName(name));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component_with_tag (Component& parent, StringConverter tag)
{
  try
  {
    return find_component_with_filter(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with tag \""+tag.str()+"\" not found in " + parent.uri().string());
  }
}

inline ComponentReference<Component const>::type
find_component_with_tag (const Component& parent, StringConverter tag)
{
  try
  {
    return find_component_with_filter(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with tag \""+tag.str()+"\" not found in " + parent.uri().string());
  }
}

template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_with_tag (ParentT& parent, StringConverter tag)
{
  try
  {
    return find_component_with_filter<ComponentT>(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component with tag \""+tag.str()+"\" and type " + ComponentT::type_name() + " not found in " + parent.uri().string());
  }

}

inline ComponentHandle<Component>::type
find_component_ptr_with_tag (Component& parent, StringConverter tag)
{
  return find_component_ptr_with_filter(parent,IsComponentTag(tag));
}

inline ComponentHandle<Component const>::type
find_component_ptr_with_tag (const Component& parent, StringConverter tag)
{
  return find_component_ptr_with_filter(parent,IsComponentTag(tag));
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_with_tag (ParentT& parent, StringConverter tag)
{
  return find_component_ptr_with_filter<ComponentT>(parent,IsComponentTag(tag));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component_recursively (Component& parent )
{
  ComponentIteratorRangeSelector<Component>::type r = find_components_recursively(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

inline ComponentReference<Component const>::type
find_component_recursively (const Component& parent )
{
  ComponentIteratorRangeSelector<Component const>::type r = find_components_recursively(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename ComponentT, typename ParentT >
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_recursively (ParentT& parent )
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type r = find_components_recursively<ComponentT>(parent);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found recursively with type " + ComponentT::type_name() + " in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found recursively with type " + ComponentT::type_name() + " in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

inline ComponentHandle<Component>::type
find_component_ptr_recursively (Component& parent)
{
  ComponentIteratorRangeSelector<Component>::type r = find_components_recursively(parent);
  typedef ComponentHandle<Component>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

inline ComponentHandle<Component const>::type
find_component_ptr_recursively (const Component& parent)
{
  ComponentIteratorRangeSelector<Component const>::type r = find_components_recursively(parent);
  typedef ComponentHandle<Component const>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_recursively (ParentT& parent)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT>::type r = find_components_recursively<ComponentT>(parent);
  typedef typename ComponentHandle<ParentT, ComponentT>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

//////////////////////////////////////////////////////////////////////////////

template<typename Predicate>
inline ComponentReference<Component>::type
find_component_recursively_with_filter(Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type r = find_components_recursively_with_filter(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found recursively with filter in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found recursively with filter in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename Predicate>
inline ComponentReference<Component const>::type
find_component_recursively_with_filter(const Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type r = find_components_recursively_with_filter(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found recursively wth filter in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found recursively with filter in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_recursively_with_filter(ParentT& parent, const Predicate& pred )
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT, Predicate>::type r = find_components_recursively_with_filter<ComponentT>(parent, pred);
  if(r.begin() == r.end())
    throw ValueNotFound(FromHere(), "Component not found recursively with filter and with type " + ComponentT::type_name() + " in " + parent.uri().string() + " : 0 matches");
  else if(count(r) > 1)
    throw ValueNotFound(FromHere(), "Component not found recursively with filter and with type " + ComponentT::type_name() + " in " + parent.uri().string() + " : more than 1 match");
  else
    return *r.begin();
}

template<typename Predicate>
inline ComponentHandle<Component>::type
find_component_ptr_recursively_with_filter(Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component, Component, Predicate>::type r = find_components_recursively_with_filter(parent, pred);
  typedef ComponentHandle<Component>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename Predicate>
inline ComponentHandle<Component const>::type
find_component_ptr_recursively_with_filter(const Component& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<Component const, Component, Predicate>::type r = find_components_recursively_with_filter(parent, pred);
  typedef ComponentHandle<Component const>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

template<typename ComponentT, typename ParentT, typename Predicate>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_recursively_with_filter(ParentT& parent, const Predicate& pred)
{
  typename ComponentIteratorRangeSelector<ParentT, ComponentT, Predicate>::type r = find_components_recursively_with_filter<ComponentT>(parent, pred);
  typedef typename ComponentHandle<ParentT, ComponentT>::type ResultT;
  if(r.begin() == r.end() || count(r) > 1)
    return ResultT();
  return r.begin().base().get();
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component_recursively_with_name(Component& parent, StringConverter name)
{
  try
  {
    return find_component_recursively_with_filter(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with name " +name.str()+ " in " + parent.uri().string());
  }
}

inline ComponentReference<Component const>::type
find_component_recursively_with_name(const Component& parent, StringConverter name)
{
  try
  {
    return find_component_recursively_with_filter(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with name " +name.str()+ " in " + parent.uri().string());
  }
}

template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_recursively_with_name(ParentT& parent, StringConverter name)
{
  try
  {
    return find_component_recursively_with_filter<ComponentT>(parent, IsComponentName(name));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with name \"" +name.str()+ "\" and with type " + ComponentT::type_name() + " in " + parent.uri().string());
  }
}

inline ComponentHandle<Component>::type
find_component_ptr_recursively_with_name(Component& parent, StringConverter name)
{
  return find_component_ptr_recursively_with_filter(parent,IsComponentName(name));
}

inline ComponentHandle<Component const>::type
find_component_ptr_recursively_with_name(const Component& parent, StringConverter name)
{
  return find_component_ptr_recursively_with_filter(parent,IsComponentName(name));
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_recursively_with_name(ParentT& parent, StringConverter name)
{
  return find_component_ptr_recursively_with_filter<ComponentT>(parent,IsComponentName(name));
}

//////////////////////////////////////////////////////////////////////////////

inline ComponentReference<Component>::type
find_component_recursively_with_tag(Component& parent, StringConverter tag)
{
  try
  {
    return find_component_recursively_with_filter(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with tag \"" +tag.str()+ "\" in " + parent.uri().string());
  }
}

inline ComponentReference<Component const>::type
find_component_recursively_with_tag(const Component& parent, StringConverter tag)
{
  try
  {
    return find_component_recursively_with_filter(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with tag \"" +tag.str()+ "\" in " + parent.uri().string());
  }
}

template<typename ComponentT, typename ParentT>
inline typename ComponentReference<ParentT, ComponentT>::type
find_component_recursively_with_tag(ParentT& parent, StringConverter tag)
{
  try
  {
    return find_component_recursively_with_filter<ComponentT>(parent, IsComponentTag(tag));
  }
  catch (ValueNotFound& e)
  {
    throw ValueNotFound(FromHere(), "Unique component not found recursively with tag \"" +tag.str()+ "\" and with type " + ComponentT::type_name() + " in " + parent.uri().string());
  }
}

inline ComponentHandle<Component>::type
find_component_ptr_recursively_with_tag(Component& parent, StringConverter tag)
{
  return find_component_ptr_recursively_with_filter(parent,IsComponentTag(tag));
}

inline ComponentHandle<Component const>::type
find_component_ptr_recursively_with_tag(const Component& parent, StringConverter tag)
{
  return find_component_ptr_recursively_with_filter(parent,IsComponentTag(tag));
}

template<typename ComponentT, typename ParentT>
inline typename ComponentHandle<ParentT, ComponentT>::type
find_component_ptr_recursively_with_tag(ParentT& parent, StringConverter tag)
{
  return find_component_ptr_recursively_with_filter<ComponentT>(parent,IsComponentTag(tag));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename ParentT, typename ComponentT, typename Predicate>
ParentT& find_parent_component_with_filter(ComponentT& comp, const Predicate& pred)
{
  Handle<Component> parent = comp.parent();
  while(is_not_null(parent))
  {
    if ( pred(parent) && IsComponentType<ParentT>()(parent) )
      return dynamic_cast<ParentT&>(*parent);

    parent = parent->parent();
  }

  throw ValueNotFound (FromHere(), "Parent of component ["+comp.uri().path()+"] with filter is not found recursively");
}

template <typename ParentT, typename ComponentT, typename Predicate>
Handle<ParentT> find_parent_component_ptr_with_filter(ComponentT& comp, const Predicate& pred)
{
  Handle<Component> parent = comp.parent();
  while(is_not_null(parent))
  {
    if ( pred(parent) && IsComponentType<ParentT>()(parent) )
      return Handle<ParentT>(parent);

    parent = parent->parent();
  }

  throw ValueNotFound (FromHere(), "Parent of component ["+comp.uri().path()+"] with filter is not found recursively");
}

template <typename ComponentT, typename Predicate>
Component& find_parent_component_with_filter(ComponentT& comp, const Predicate& pred)
{
  return find_parent_component_with_filter<Component>(comp,pred);
}

template <typename ComponentT, typename Predicate>
Handle<Component> find_parent_component_ptr_with_filter(ComponentT& comp, const Predicate& pred)
{
  return find_parent_component_ptr_with_filter<Component>(comp,pred);
}

////////////////////////////////////////////////////////////////////////////////

template <typename ParentT, typename ComponentT>
ParentT& find_parent_component(ComponentT& comp)
{
  return find_parent_component_with_filter<ParentT>(comp,IsComponentTrue());
}

template <typename ParentT, typename ComponentT>
Handle<ParentT> find_parent_component_ptr(ComponentT& comp)
{
  return find_parent_component_ptr_with_filter<ParentT>(comp,IsComponentTrue());
}

////////////////////////////////////////////////////////////////////////////////

template <typename ParentT, typename ComponentT>
ParentT& find_parent_component_with_name(ComponentT& comp, StringConverter name)
{
  return find_parent_component_with_filter<ParentT>(comp,IsComponentName(name));
}

template <typename ParentT, typename ComponentT>
Handle<ParentT> find_parent_component_with_name(ComponentT& comp, StringConverter name)
{
  return find_parent_component_ptr_with_filter<ParentT>(comp,IsComponentName(name));
}

template <typename ComponentT>
Component& find_parent_component_with_name(ComponentT& comp, StringConverter name)
{
  return find_parent_component_with_filter(comp,IsComponentName(name));
}

template <typename ComponentT>
Handle<Component> find_parent_component_ptr_with_name(ComponentT& comp, StringConverter name)
{
  return find_parent_component_ptr_with_filter(comp,IsComponentName(name));
}

////////////////////////////////////////////////////////////////////////////////

template <typename ParentT, typename ComponentT>
ParentT& find_parent_component_with_tag(ComponentT& comp, StringConverter tag)
{
  return find_parent_component_with_filter<ParentT>(comp,IsComponentTag(tag));
}

template <typename ParentT, typename ComponentT>
Handle<ParentT> find_parent_component_ptr_with_tag(ComponentT& comp, StringConverter tag)
{
  return find_parent_component_ptr_with_filter<ParentT>(comp,IsComponentTag(tag));
}

template <typename ComponentT>
Component& find_parent_component_with_tag(ComponentT& comp, StringConverter tag)
{
  return find_parent_component_with_filter(comp,IsComponentTag(tag));
}

template <typename ComponentT>
Handle<Component> find_parent_component_ptr_with_tag(ComponentT& comp, StringConverter tag)
{
  return find_parent_component_ptr_with_filter(comp,IsComponentTag(tag));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_FindComponents_hpp
