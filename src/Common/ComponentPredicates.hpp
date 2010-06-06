#ifndef CF_Common_ComponentPredicates_hpp
#define CF_Common_ComponentPredicates_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/filter_iterator.hpp>
#include <boost/foreach.hpp>

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
  IsComponentType (Predicate pred) : m_type(CType::getClassName()), m_pred(pred) {}
  IsComponentType () : m_type(CType::getClassName()), m_pred() {}

  bool operator()(const Component::Ptr& component)
  { return boost::bind( &Component::has_tag , _1 , m_type )(component) && m_pred(component); }

  bool operator()(const Component& component)
  { return boost::bind( &Component::has_tag , _1 , m_type )(component) && m_pred(component); }

};

////////////////////////////////////////////////////////////////////////////////
// Wrappers to make iterating easy
////////////////////////////////////////////////////////////////////////////////

template <typename Predicate, typename IteratorT>
inline boost::iterator_range<boost::filter_iterator<Predicate, IteratorT> >
iterate_recursive(const IteratorT& from, const IteratorT& to , const Predicate& pred, Uint level=0)
{
  return boost::make_iterator_range(boost::filter_iterator<Predicate, IteratorT >(pred,from,to),
                                    boost::filter_iterator<Predicate, IteratorT >(pred,to,to));
}

template <typename IteratorT>
inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, IteratorT > >
iterate_recursive(const IteratorT& from, const IteratorT& to, Uint level=0)
{
  return iterate_recursive(from,to,IsComponentTrue(),level);
}

////////////////////////////////////////////////////////////////////////////////

template <typename CType, typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component::iterator > >
iterate_recursive(CType& parent, const Predicate& pred, Uint level=0)
{
  return iterate_recursive(parent.template recursive_begin<Component>(),parent.template recursive_end<Component>(),pred,level);
}

template <typename CType>
inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, Component::iterator> >
iterate_recursive(CType& parent, Uint level=0)
{
  return iterate_recursive(parent,IsComponentTrue(),level);
}

////////////////////////////////////////////////////////////////////////////////

/// Recursive range over all contained components of type CReturnType
/// and complying to the supplied predicate
template <typename CReturnType, typename CType, typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component_iterator<CReturnType> > >
iterate_recursive_by_type(CType& parent, const Predicate& pred, Uint level=0)
{
  return iterate_recursive(parent.template recursive_begin<CReturnType>(),parent.template recursive_end<CReturnType>(),pred,level);
}

/// Recursive range over all contained components of type CReturnType
template <typename CReturnType, typename CType>
inline boost::iterator_range<Component_iterator<CReturnType> >
iterate_recursive_by_type(CType& parent, Uint level=0)
{
  return boost::make_iterator_range(parent.template recursive_begin<CReturnType>(), parent.template recursive_end<CReturnType>());
}

/// Recursive range over all contained components of type CReturnType
/// and complying to the supplied predicate
/// Const version
template <typename CReturnType, typename CType, typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component_iterator<CReturnType const> > >
iterate_recursive_by_type(const CType& parent, const Predicate& pred, Uint level=0)
{
  return iterate_recursive(parent.template recursive_begin<CReturnType>(),parent.template recursive_end<CReturnType>(),pred,level);
}

/// Recursive range over all contained components of type CReturnType
/// Const version
template <typename CReturnType, typename CType>
inline boost::iterator_range<Component_iterator<CReturnType const> >
iterate_recursive_by_type(const CType& parent, Uint level=0)
{
  return boost::make_iterator_range(parent.template recursive_begin<CReturnType>(), parent.template recursive_end<CReturnType>());
}

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
