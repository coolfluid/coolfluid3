// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Component_hpp
#define CF_Common_Component_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/enable_shared_from_this.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/SignalHandler.hpp"
#include "Common/TaggedObject.hpp"
#include "Common/CPath.hpp"
#include "Common/ComponentIterator.hpp"
#include "Common/XmlHelpers.hpp"

namespace CF {
namespace Common {

  class XmlParams;
  class Option;
  class CRoot;

////////////////////////////////////////////////////////////////////////////////

/// Base class for defining CF components
/// @author Tiago Quintino
/// @author Willem Deconinck
class Common_API Component
  :
  public boost::enable_shared_from_this<Component>,
  public ConfigObject,
  public SignalHandler,
  public TaggedObject,
  public boost::noncopyable {

public: // typedef

  /// type of pointer to Component
  typedef boost::shared_ptr<Component> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<Component const> ConstPtr;

  /// type of the iterator to Component
  typedef ComponentIterator<Component> iterator;
  /// type of the iterator to constant Component
  typedef ComponentIterator<Component const> const_iterator;

private: // typedef

  /// type for storing the sub components
  typedef std::map < std::string , Component::Ptr > CompStorage_t;

public: // functions

  /// Get the class name
  static std::string type_name () { return "Component"; }

  /// Contructor
  /// @param name of the component
  /// @param parent path where this component will be placed
  Component ( const std::string& name );

  /// Virtual destructor
  virtual ~Component();

  /// Get the component through the links to the actual components
  virtual Component::Ptr  get();

  virtual Component::ConstPtr  get() const;

  /// @return a shared pointer to self
  Component::Ptr self() { return shared_from_this(); }

  /// @name ITERATORS
  //@{

  /// The begin iterator for a range containing only components of the specified type
  template<typename ComponentT>
  ComponentIterator<ComponentT> begin();

  /// The begin iterator for a range containing Components
  Component::iterator begin();

  /// The end iterator for a range containing only components of the specified type
  template<typename ComponentT>
  ComponentIterator<ComponentT> end();

  /// The end iterator for a range containing Components
  Component::iterator end();

  /// The begin iterator for a range containing only components of the specified type (const version)
  template<typename ComponentT>
  ComponentIterator<ComponentT const> begin() const;

  /// The begin iterator for a range containing Components (const version)
  Component::const_iterator begin() const;

  /// The end iterator for a range containing only components of the specified type (const version)
  template<typename ComponentT>
  ComponentIterator<ComponentT const> end() const;

  /// The end iterator for a range containing Components (const version)
  Component::const_iterator end() const;

  /// The begin iterator for a recursive range containing only components of the specified type
  template<typename ComponentT>
  ComponentIterator<ComponentT> recursive_begin();

  /// The begin iterator for a recursive range containing Components
  Component::iterator recursive_begin();

  /// The end iterator for a recursive range containing only components of the specified type
  template<typename ComponentT>
  ComponentIterator<ComponentT> recursive_end();

  /// The end iterator for a recursive range containing Components
  Component::iterator recursive_end();

  /// The begin iterator for a recursive range containing only components of the specified type (const version)
  template<typename ComponentT>
  ComponentIterator<ComponentT const> recursive_begin() const;

  /// The begin iterator for a recursive range containing Components (const version)
  Component::const_iterator recursive_begin() const;

  /// The end iterator for a recursive range containing only components of the specified type (const version)
  template<typename ComponentT>
  ComponentIterator<ComponentT const> recursive_end() const;

  /// The end iterator for a recursive range containing Components (const version)
  Component::const_iterator recursive_end() const;

  //@} END ITERATORS

  /// checks if this component is in fact a link to another component
  bool is_link () const { return m_is_link; }

  /// Access the name of the component
  std::string name () const { return m_name.string(); }

  /// Rename the component
  void rename ( const std::string& name );

  /// Access the path of the component
  const CPath& path () const { return m_path; }

  /// Construct the full path
  CPath full_path () const { return m_path / m_name; }

  /// Resolves relative elements within a path to complete it.
  /// The path may be relative to this component or absolute.
  /// This is strictly a path operation so the path may not actually point anywhere
  /// @param path to a component
  /// @post path statisfies CPath::is_complete()
  /// @post path statisfies CPath::is_absolute()
  void complete_path ( CPath& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component
  ConstPtr look_component ( const CPath& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component
  Ptr look_component ( const CPath& path );

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component cast to specific type
  template < typename T >
    typename T::ConstPtr look_component_type ( const CPath& path ) const
  {
    return boost::dynamic_pointer_cast<T const>(look_component(path));
  }

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component cast to specific type
  template < typename T >
  typename T::Ptr look_component_type ( const CPath& path )
  {
    return boost::dynamic_pointer_cast<T>(look_component(path));
  }

  /// @returns the pointer to parent component
  /// @pre parent pointer is valid
  Ptr get_parent();
  /// @returns the const pointer to parent component
  /// @pre parent pointer is valid
  ConstPtr get_parent() const;

  /// Get the named child from the direct subcomponents.
  Ptr get_child(const std::string& name);
  ConstPtr get_child(const std::string& name) const;

  /// @returns the named child from the direct subcomponents automatically cast to the specified type
  template < typename T >
      typename T::Ptr get_child_type ( const std::string& name );

  /// @returns the named child from the direct subcomponents automatically cast to the specified type
  template < typename T >
      typename T::ConstPtr get_child_type ( const std::string& name ) const ;

  /// @returns this component converted to type T shared pointer
  template < typename T >
    typename T::Ptr get_type();

  /// @returns this component converted to type T shared const pointer
  template < typename T >
    typename T::ConstPtr get_type() const;

  /// Modify the parent of this component
  void change_parent ( Component* to_parent );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
      typename T::Ptr create_component_type ( const std::string& name );

  /// Add a dynamic (sub)component of this component
  Ptr add_component ( Ptr subcomp );

  /// Remove a (sub)component of this component
  Ptr remove_component ( const std::string& name );

  /// Move this component to within another one
  /// @param to_parent will be the new parent of this component
  void move_component ( Ptr to_parent );

  /// @returns a string representation of the tree below this component
  std::string tree(Uint level=0) const;

  /// @return Returns the number of children this component has.
  size_t get_child_count() const;

  /// @return Returns the type name of the subclass, according to
  /// @c CF::TypeInfo
  virtual std::string derived_type_name() const
  {
    return CF::TypeInfo::instance().portable_types[ typeid(*this).name() ];
  }

  /// @return Returns a reference to the property list
  PropertyList & properties() { return m_properties; }

  /// @return Returns a constant referent to the property list
  const PropertyList & properties() const { return m_properties; }

  /// @name SIGNALS
  //@{

  /// creates a component from this component
  void create_component ( XmlNode& xml );

  /// lists the sub components and puts them on the xml_tree
  void list_tree ( XmlNode& xml );

  /// lists the properties of this component
  void list_properties ( XmlNode& xml );

  /// lists the signals of this component
  void list_signals ( XmlNode& xml );

  /// renames this component
  void rename_component ( XmlNode& xml) ;

  //@} END SIGNALS

  /// marks this component as basic.
  void mark_basic();

protected: // functions

  /// Add a static (sub)component of this component
  Ptr add_static_component ( Ptr subcomp );

  /// Tags the
  template < typename TYPE >
  void tag_component ( TYPE* self ) { self->add_tag( TYPE::type_name() ); }

private: // helper functions

  /// insures the sub component has a unique name within this component
  std::string ensure_unique_name ( Ptr subcomp );

  /// writes the underlying component tree to the xml node
  void write_xml_tree( XmlNode& node );

  /// Put all subcomponents in a given vector, optionally recursive
  /// @param [out] vec  A vector of all (recursive) subcomponents
  /// @param [in] recurse If true, recurse through all subcomponents. If false, puts only direct children
  template<typename ComponentT>
  void put_components(std::vector<typename ComponentT::Ptr>& vec, const bool recurse);

  template<typename ComponentT>
  void put_components(std::vector<boost::shared_ptr<ComponentT const> >& vec, const bool recurse) const;

  /// Returns an iterator
  /// @param [in] begin If true, the begin iterator is returned, otherwise end
  /// @param [out] recursive If true, the iterator recurses over all components below this
  template<typename ComponentT>
  ComponentIterator<ComponentT> make_iterator(const bool begin, const bool recursive);

  template<typename ComponentT>
  ComponentIterator<ComponentT const> make_iterator(const bool begin, const bool recursive) const;

protected: // data

  /// component name (stored as path to ensure validity)
  CPath m_name;
  /// component current path
  CPath m_path;
  /// list of sub-components
  CompStorage_t m_components;
  /// list of dynamic sub-components
  CompStorage_t m_dynamic_components;
  /// pointer to the root of this tree
  boost::weak_ptr<CRoot> m_root;
  /// pointer to parent, naked pointer because of static components
  Component * m_raw_parent;
  /// is this a link component
  bool m_is_link;

  void raise_path_changed();

  void raise_event(const std::string & name );

}; // Component

////////////////////////////////////////////////////////////////////////////////

/// Stand-alone function to allocate components of a given type
template < typename T >
boost::shared_ptr<T> allocate_component_type ( const std::string& name )
{
  typename boost::shared_ptr<T> comp ( new T(name), Deleter<T>() );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_component_type ( const std::string& name )
{
  typename T::Ptr comp = allocate_component_type<T>(name);
  add_component( comp );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::get_child_type(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T>(found->second);
  return typename T::Ptr();
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::ConstPtr Component::get_child_type(const std::string& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T const>(found->second);
  return typename T::ConstPtr();
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::get_type()
{
  return boost::dynamic_pointer_cast<T>(get());
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::ConstPtr Component::get_type() const
{
  return boost::dynamic_pointer_cast<T const>(get());
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline void Component::put_components(std::vector<typename ComponentT::Ptr >& vec, const bool recurse)
{
  for(CompStorage_t::iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    if(typename ComponentT::Ptr p = boost::dynamic_pointer_cast<ComponentT>(it->second))
    {
      vec.push_back(p);
    }
    if(recurse)
      it->second->put_components<ComponentT>(vec, true);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
void Component::put_components(std::vector<boost::shared_ptr<ComponentT const> >& vec, const bool recurse) const
{
  for(CompStorage_t::const_iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    if(boost::shared_ptr<ComponentT const> p = boost::dynamic_pointer_cast<ComponentT const>(it->second))
    {
      vec.push_back(p);
    }
    if(recurse)
      it->second->put_components<ComponentT>(vec, true);
  }
}

////////////////////////////////////////////////////////////////////////////////

// specialization avoiding the dynamic cast
template<>
inline void Component::put_components<Component>(std::vector<Component::Ptr>& vec, const bool recurse)
{
  for(CompStorage_t::iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    vec.push_back(it->second);
    if(recurse)
      it->second->put_components<Component>(vec, true);
  }
}

template<>
inline void Component::put_components<Component>(std::vector<boost::shared_ptr<Component const> >& vec, const bool recurse) const
{
  for(CompStorage_t::const_iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    vec.push_back(it->second);
    if(recurse)
      it->second->put_components<Component>(vec, true);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::make_iterator(const bool begin, const bool recursive)
{
  std::vector<boost::shared_ptr<ComponentT> > vec;
  put_components<ComponentT>(vec, recursive);
  return ComponentIterator<ComponentT>(vec, begin ? 0 : vec.size());
}

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::make_iterator(const bool begin, const bool recursive) const
{
  std::vector<boost::shared_ptr<ComponentT const> > vec;
  put_components<ComponentT>(vec, recursive);
  return ComponentIterator<ComponentT const>(vec, begin ? 0 : vec.size());
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::begin()
{
  return make_iterator<ComponentT>(true, false);
}

inline Component::iterator Component::begin()
{
  return begin<Component>();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::end()
{
  return make_iterator<ComponentT>(false, false);
}

inline Component::iterator Component::end()
{
  return end<Component>();
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::begin() const
{
  return make_iterator<ComponentT>(true, false);
}

inline Component::const_iterator Component::begin() const
{
  return begin<Component>();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::end() const
{
  return make_iterator<ComponentT>(false, false);
}

inline Component::const_iterator Component::end() const
{
  return end<Component>();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::recursive_begin()
{
  return make_iterator<ComponentT>(true, true);
}

inline Component::iterator Component::recursive_begin()
{
  return recursive_begin<Component>();
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::recursive_end()
{
  return make_iterator<ComponentT>(false, true);
}

inline Component::iterator Component::recursive_end()
{
  return recursive_end<Component>();
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::recursive_begin() const
{
  return make_iterator<ComponentT>(true, true);
}

inline Component::const_iterator Component::recursive_begin() const
{
  return recursive_begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::recursive_end() const
{
  return make_iterator<ComponentT>(false, true);
}

inline Component::const_iterator Component::recursive_end() const
{
  return recursive_end<Component>();
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_Component_hpp
