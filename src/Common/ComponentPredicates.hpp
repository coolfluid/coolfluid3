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
  IsComponentType (Predicate pred) : m_type(CType::getClassName()), m_pred(pred) {}
  IsComponentType () : m_type(CType::getClassName()), m_pred() {}

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
};

/// Specialization without the predicate and no filter
template<typename IteratorT>
struct FilteredIteratorRange<IteratorT, IsComponentTrue> {
  typedef boost::iterator_range<IteratorT> type;
};

/// Derive the correct range type based on the constness of ParentT, which should be the type of the parent component
template<typename ParentT, typename ComponentT=Component, typename Predicate=IsComponentTrue>
struct ComponentIteratorRange {
  typedef typename FilteredIteratorRange<typename boost::mpl::if_c<boost::is_const<ParentT>::value, // if ParentT is const
                                                                    Component_iterator<ComponentT const>, // use a const component iterator
                                                                    Component_iterator<ComponentT> >::type, // or the mutable one otherwise
                                          Predicate>::type type;
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
  return boost::make_iterator_range(boost::filter_iterator<Predicate, IteratorT>(pred,from,to),
                                    boost::filter_iterator<Predicate, IteratorT>(pred,to,to));
}


/// Creates a range containing all components of type ComponentT from the whole tree under parent, filtered by the predicate
/// Note: ParentT is a template argument instead of base class Component to allow const and non-const versions in one go
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
/// Note: ParentT is a template argument instead of base class Component to allow const and non-const versions in one go
template <typename ComponentT, typename ParentT>
inline typename ComponentIteratorRange<ParentT, ComponentT>::type
recursive_range_typed(ParentT& parent)
{
  return boost::make_iterator_range(parent.template recursive_begin<ComponentT>(),parent.template recursive_end<ComponentT>());
}

template <typename ParentT>
inline typename ComponentIteratorRange<ParentT, Component>::type
recursive_range(ParentT& parent)
{
  return recursive_range_typed<Component>(parent);
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
