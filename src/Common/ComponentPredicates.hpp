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

  bool operator()(boost::shared_ptr<Component>& component)
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

  bool operator()(boost::shared_ptr<Component>& component)
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

  bool operator()(boost::shared_ptr<Component>& component)
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

  bool operator()(Component& component)
  { return boost::bind( &Component::has_tag , _1 , m_tag )(component); }

};

////////////////////////////////////////////////////////////////////////////////
// Wrappers to make iterating easy
////////////////////////////////////////////////////////////////////////////////

template <typename CType,typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component_iterator<CType> > >
make_component_range(const Component_iterator<CType>& from, const Component_iterator<CType>& to , const Predicate& pred)
{
  return boost::make_iterator_range(boost::filter_iterator<Predicate, Component_iterator<CType> >(pred,from,to),
                                    boost::filter_iterator<Predicate, Component_iterator<CType> >(pred,to,to));
}

template <typename CType>
inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, Component_iterator<CType> > >
make_component_range(const Component_iterator<CType>& from, const Component_iterator<CType>& to)
{
  return make_component_range(from,to,IsComponentTrue());
}

////////////////////////////////////////////////////////////////////////////////

template <typename CType,typename Predicate>
inline boost::iterator_range<boost::filter_iterator<Predicate, Component_iterator<CType> > >
make_component_range(const boost::shared_ptr<CType>& parent, const Predicate& pred)
{
  return make_component_range(parent->begin(),parent->end(),pred);
}

template <typename CType>
inline boost::iterator_range<boost::filter_iterator<IsComponentTrue, Component_iterator<CType> > >
make_component_range(const boost::shared_ptr<CType>& parent)
{
  return make_component_range(parent,IsComponentTrue());
}

////////////////////////////////////////////////////////////////////////////////

template <typename CReturnType, typename CType>
inline boost::iterator_range<boost::filter_iterator<IsComponentTag, Component_iterator<CType> > >
make_component_range_of_type(const boost::shared_ptr<CType>& parent)
{
  return make_component_range(parent,IsComponentTag(CReturnType::getClassName()));
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ComponentPredicates_hpp
