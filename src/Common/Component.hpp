// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Component_hpp
#define CF_Common_Component_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/enable_shared_from_this.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "Common/SignalHandler.hpp"
#include "Common/TaggedObject.hpp"
#include "Common/CPath.hpp"

namespace CF {
namespace Common {

  class CRoot;

  template<class T> class ComponentIterator;

////////////////////////////////////////////////////////////////////////////////

/// Base class for defining CF components
/// @author Tiago Quintino
/// @author Willem Deconinck
class Common_API Component :
  public boost::enable_shared_from_this<Component>,
  public boost::noncopyable,
  public SignalHandler,
  public TaggedObject
{

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

  /// @return a shared pointer to self
  Component::ConstPtr self() const { return shared_from_this(); }

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
  /// @throws ValueNotFound if the component is not found in the path
  /// @post returns empty shared pointer if failed to cast to requested type
  /// @return constant Ptr to component cast to specific type
  /// @todo to be replaced by look_component<T>
  template < typename T >
    typename T::ConstPtr look_component_type ( const CPath& path ) const
  {
    return boost::dynamic_pointer_cast<T const>(look_component(path));
  }
  template < typename T >
    typename T::ConstPtr look_component ( const CPath& path ) const
  {
    return boost::dynamic_pointer_cast<T const>(look_component(path));
  }

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component cast to specific type
  /// @todo to be replaced by look_component<T>
  template < typename T >
  typename T::Ptr look_component_type ( const CPath& path )
  {
    return boost::dynamic_pointer_cast<T>(look_component(path));
  }
  template < typename T >
  typename T::Ptr look_component ( const CPath& path )
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
  /// @todo to be replaced by get_child<T>
  template < typename T >
      typename T::Ptr get_child_type ( const std::string& name );
  template < typename T >
      typename T::Ptr get_child ( const std::string& name );

  /// @returns the named child from the direct subcomponents automatically cast to the specified type
  /// @todo to be replaced by get_child<T>
  template < typename T >
      typename T::ConstPtr get_child_type ( const std::string& name ) const ;
  template < typename T >
      typename T::ConstPtr get_child ( const std::string& name ) const ;

  /// @returns this component converted to type T shared pointer
  template < typename T >
    typename T::Ptr as_type();

  /// @returns this component converted to type T shared const pointer
  template < typename T >
    typename T::ConstPtr as_type() const;

  /// Modify the parent of this component
  void change_parent ( Component* to_parent );

  /// Create a (sub)component of this component automatically cast to the specified type
  /// @todo replace all used instances by create_component<T>()
  template < typename T >
      typename T::Ptr create_component_type ( const std::string& name );
  template < typename T >
      typename T::Ptr create_component ( const std::string& name );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
      typename T::Ptr create_static_component ( const std::string& name );

  /// Add a dynamic (sub)component of this component
  Ptr add_component ( Ptr subcomp );

  /// Remove a (sub)component of this component
  Ptr remove_component ( const std::string& name );

  /// Move this component to within another one
  /// @param to_parent will be the new parent of this component
  void move_to ( Ptr to_parent );

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
  PropertyList& properties() { return m_properties; }

  /// @return Returns a constant referent to the property list
  const PropertyList& properties() const { return m_properties; }

  /// access to the property
  const Property& property(const std::string& optname ) const;

  /// Configure one property, and trigger its actions
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  void configure_property(const std::string& optname, const boost::any& val)
  {
    m_properties.configure_property(optname,val);
  }

  /// @name SIGNALS
  //@{

  /// configures all the options on this class
  void configure ( XmlNode& xml );

  /// creates a component from this component
  void create_component_signal ( XmlNode& xml );

  /// deletes a component from this component
  void delete_component ( XmlNode& xml );

  /// moves a component from this component to another
  void move_component ( XmlNode& xml );

  /// lists the sub components and puts them on the xml_tree
  void list_tree ( XmlNode& xml );

  /// lists the properties of this component
  void list_properties ( XmlNode& xml );

  /// lists the signals of this component
  void list_signals ( XmlNode& xml );

  ///  gets info on this component
  void print_info ( XmlNode& xml );

  /// renames this component
  void rename_component ( XmlNode& xml) ;

  //@} END SIGNALS

  /// marks this component as basic.
  void mark_basic();

protected: // functions

  /// Add a static (sub)component of this component
  Ptr add_static_component ( Ptr subcomp );

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
  /// storage of the option list
  PropertyList m_properties;
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

template<class T>
class ComponentIterator
        : public boost::iterator_facade<ComponentIterator<T>,        // iterator
                                        T,                            // Value
                                        boost::bidirectional_traversal_tag, // search direction
                                        T&                            // return type of dereference is a reference
                                       >
{
  typedef boost::iterator_facade<ComponentIterator<T>, T, boost::random_access_traversal_tag, T&> BaseT;
public:

  typedef typename BaseT::difference_type difference_type;

  /// Construct an iterator over the given set of components. If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ComponentIterator(std::vector<boost::shared_ptr<T> > vec, const Uint startPosition)
          : m_vec(vec), m_position(startPosition)
  {
  }

private:
  friend class boost::iterator_core_access;
  template <class> friend class ComponentIterator;

  template <typename T2>
  bool equal(ComponentIterator<T2> const& other) const { return (m_position == other.m_position); }

  void increment()
  {
    cf_assert(m_position != m_vec.size());
    ++m_position;
  }

  void decrement()
  {
    cf_assert(m_position != 0);
    --m_position;
  }

  void advance(const difference_type n) { m_position += n; }

  template <typename T2>
  difference_type distance_to(ComponentIterator<T2> const& other) const
  {
    return other.m_position - m_position;
  }

public:
  T& dereference() const { return *m_vec[m_position]; }

  /// Get a shared pointer to the referenced object
  boost::shared_ptr<T> get() const { return m_vec[m_position]; }

  /// Compatibility with boost filtered_iterator interface, so base() can be used transparently on all ranges
  ComponentIterator<T>& base() { return *this; }

  const ComponentIterator<T>& base() const { return *this; }

private:
  std::vector<boost::shared_ptr<T> > m_vec;
  Uint m_position;
};

////////////////////////////////////////////////////////////////////////////////

/// Stand-alone function to allocate components of a given type
/// @todo to be replaced by allocate_component<T>
template < typename T >
boost::shared_ptr<T> allocate_component_type ( const std::string& name )
{
  typename boost::shared_ptr<T> comp ( new T(name), Deleter<T>() );
  return comp ;
}

//////////////////////////////////////////////////////////////////////////////

/// Stand-alone function to allocate components of a given type
template < typename T >
boost::shared_ptr<T> allocate_component ( const std::string& name )
{
  typename boost::shared_ptr<T> comp ( new T(name), Deleter<T>() );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////

/// @todo to be replaced by create_component<T>
template < typename T >
inline typename T::Ptr Component::create_component_type ( const std::string& name )
{
  typename T::Ptr comp = allocate_component<T>(name);
  add_component( comp );
  return comp ;
}

//////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_component ( const std::string& name )
{
  typename T::Ptr comp = allocate_component<T>(name);
  add_component( comp );
  return comp ;
}

//////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_static_component ( const std::string& name )
{
  typename T::Ptr comp = allocate_component<T>(name);
  add_static_component( comp );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////

/// @todo to be replaced by get_child<T>
template < typename T >
inline typename T::Ptr Component::get_child_type(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T>(found->second);
  return typename T::Ptr();
}

////////////////////////////////////////////////////////////////////////////////

/// @todo to be replaced by get_child<T>
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
inline typename T::Ptr Component::get_child(const std::string& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T>(found->second);
  return typename T::Ptr();
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::ConstPtr Component::get_child(const std::string& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T const>(found->second);
  return typename T::ConstPtr();
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::as_type()
{
  return boost::dynamic_pointer_cast<T>(get());
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::ConstPtr Component::as_type() const
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
