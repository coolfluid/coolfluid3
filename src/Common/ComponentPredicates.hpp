#ifndef CF_Common_ComponentPredicates_hpp
#define CF_Common_ComponentPredicates_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/iterator/filter_iterator.hpp>

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

  bool operator()(const Component::Ptr& component)
  { return true; }

  bool operator()(Component& component)
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

  bool operator()(Component& component)
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

  bool operator()(Component& component)
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

};

////////////////////////////////////////////////////////////////////////////////
// Wrappers to make iterating easy
////////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component::iterator> >
iterate_recursive(const Component::iterator& from, const Component::iterator& to , const Predicate& pred, Uint level=0)
{
  return boost::make_iterator_range(boost::filter_iterator<Predicate, Component::iterator >(pred,from,to),
                                    boost::filter_iterator<Predicate, Component::iterator >(pred,to,to));
}

inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, Component::iterator > >
iterate_recursive(const Component::iterator& from, const Component::iterator& to, Uint level=0)
{
  return iterate_recursive(from,to,IsComponentTrue(),level);
}

////////////////////////////////////////////////////////////////////////////////

template <typename CType, typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component::iterator > >
iterate_recursive(const boost::shared_ptr<CType>& parent, const Predicate& pred, Uint level=0)
{
  return iterate_recursive(parent->begin(),parent->end(),pred,level);
}

template <typename CType>
inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, Component::iterator> >
iterate_recursive(const boost::shared_ptr<CType>& parent, Uint level=0)
{
  return iterate_recursive(parent,IsComponentTrue(),level);
}

////////////////////////////////////////////////////////////////////////////////

template <typename CReturnType, typename CType>
inline std::vector<typename CReturnType::Ptr >
iterate_recursive_by_type(const boost::shared_ptr<CType>& parent, Uint level=0)
{
  std::vector<boost::shared_ptr<CReturnType> > vec;
  BOOST_FOREACH(const Component::Ptr& component, iterate_recursive(parent,IsComponentTag(CReturnType::getClassName()),level) )
    vec.push_back(boost::dynamic_pointer_cast<CReturnType>(component));

  return vec;
}

////////////////////////////////////////////////////////////////////////////////

//original
//    template <typename CReturnType, typename CType>
//    inline boost::iterator_range<boost::filter_iterator<IsComponentTag, Component_iterator<CType> > >
//    make_component_range_of_type(const boost::shared_ptr<CType>& parent)
//    {
//      return make_component_range(parent,IsComponentTag(CReturnType::getClassName()));
//    }

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentPredicates_hpp
