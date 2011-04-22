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

#include "Common/Assertions.hpp"

#include "Common/PropertyList.hpp"
#include "Common/SignalHandler.hpp"
#include "Common/TaggedObject.hpp"
#include "Common/URI.hpp"

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
  virtual Component::Ptr  follow();

  virtual Component::ConstPtr  follow() const;

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

  /// checks if the child component with name is static
  bool is_child_static ( const std::string& name ) const;
  
  /// Access the name of the component
  std::string name () const { return m_name.path(); }

  /// Rename the component
  void rename ( const std::string& name );

  /// Access the path of the component
  const URI& path () const { return m_path; }
  /// Construct the full path
  URI full_path () const { return m_path / m_name; }

  /// Resolves relative elements within a path to complete it.
  /// The path may be relative to this component or absolute.
  /// This is strictly a path operation so the path may not actually point anywhere
  /// @param path to a component
  /// @post path statisfies URI::is_complete()
  /// @post path statisfies URI::is_absolute()
  void complete_path ( URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @throws InvalidURI in case a component is not found at that path
  /// @return reference to component
  Component& access_component ( const URI& path );

  /// Looks for a component via its path
  /// @param path to the component
  /// @return reference to component
  const Component& access_component ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  ConstPtr access_component_ptr ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  ConstPtr access_component_ptr_checked ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  Ptr access_component_ptr ( const URI& path );

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  Ptr access_component_ptr_checked ( const URI& path );

  /// @returns the pointer to parent component
  /// @pre parent pointer is valid
  /// @post returns always valid pointer
  Ptr parent();
  /// @returns the const pointer to parent component
  /// @pre parent pointer is valid
  /// @post returns always valid pointer
  ConstPtr parent() const;

  /// Gets the named child component from the list of direct subcomponents.
  /// @return reference to the component
  Component& get_child(const std::string& name);
  /// Gets the named child component from the list of direct subcomponents.
  /// @post pointer may be null
  /// @return shared pointer to the component
  Ptr get_child_ptr(const std::string& name);
  /// Gets the named child component from the list of direct subcomponents.
  /// @post pointer may be null
  /// @return const shared pointer to the component
  ConstPtr get_child_ptr(const std::string& name) const;
  /// Gets the named child component from the list of direct subcomponents.
  /// @throws ValueNotFound in case a component with given name is not found
  /// @post pointer is never null
  /// @return shared pointer to the component
  Ptr get_child_ptr_checked(const std::string& name);

  /// @returns this component converted to type T shared pointer
  template < typename T > boost::shared_ptr<T> as_ptr();
  /// @returns this component converted to type T shared const pointer
  template < typename T > boost::shared_ptr<T const> as_ptr() const;

  /// @returns this component converted to type T shared pointer
  template < typename T > boost::shared_ptr<T> as_ptr_checked();
  /// @returns this component converted to type T shared const pointer
  template < typename T > typename boost::shared_ptr<T const> as_ptr_checked() const;

  /// @returns this component converted to type T reference
  template < typename T > T& as_type() { return * as_ptr_checked<T>(); }
  /// @returns this component converted to type T reference
  template < typename T > const T& as_type() const { return * as_ptr_checked<T>(); }

  /// @return a ConstPtr
  ConstPtr as_const() const;

  Ptr as_non_const() const;

  /// Create a (sub)component of this component automatically cast to the specified type
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
  size_t count_children() const;

  /// @return Returns the type name of the subclass, according to
  /// @c CF::Common::TypeInfo
  virtual std::string derived_type_name() const;

  /// @return Returns a reference to the property list
  PropertyList& properties() { return m_properties; }

  /// @return Returns a constant referent to the property list
  const PropertyList& properties() const { return m_properties; }

  /// access to the property
  const Property& property(const std::string& optname ) const;

  Property& property(const std::string& optname );

  /// Configure one property, and trigger its actions
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  Component& configure_property(const std::string& optname, const boost::any& val);

  void configure_option_recursively(const std::string& tag, const boost::any& val);

  std::string option_list();
  
  /// Configures all the options on this class from a list of strings.
  /// Each string provides the configuration of one property following the
  /// format var_name:type=value var_name:array[type]=val1,val2
  void configure (const std::vector<std::string>& args);

  /// @name SIGNALS
  //@{

  /// configures all the options on this class
  void signal_configure ( SignalArgs& args );

  /// creates a component from this component
  void signal_create_component ( SignalArgs& xml );

  /// deletes a component from this component
  void signal_delete_component ( SignalArgs& args );

  /// moves a component from this component to another
  void signal_move_component ( SignalArgs& args );

  /// lists the sub components and puts them on the xml_tree
  void signal_list_tree( SignalArgs& args );

  /// lists the properties of this component
  void signal_list_properties ( SignalArgs& args );

  /// lists the signals of this component
  void signal_list_signals ( SignalArgs& args );

  ///  gets info on this component
  void signal_print_info ( SignalArgs& args );

  /// renames this component
  void signal_rename_component ( SignalArgs& args ) ;

  /// dumps the tree to a file
  void signal_save_tree ( SignalArgs& args );

  /// gives information about this component such as options, signals, ...
  void signal_list_content( SignalArgs& args );

  /// Gives a signal signature, if any
  void signal_signature( SignalArgs & args );

  /// Defines the signature of "create_component" signal.
  /// @param node The frame under which signature is added.
  void signature_create_component( SignalArgs& args );

  /// Defines the signature of "rename_component" signal.
  /// @param node The frame under which signature is added.
  void signature_rename_component( SignalArgs& args );

  /// Defines the signature of "move_component" signal.
  /// @param node The frame under which signature is added.
  void signature_move_component( SignalArgs& args );

  //@} END SIGNALS

  /// marks this component as basic.
  Component& mark_basic();

protected: // functions

  /// Add a static (sub)component of this component
  Ptr add_static_component ( Ptr subcomp );

private: // helper functions

  /// Modify the parent of this component
  void change_parent ( Component* to_parent );

  /// insures the sub component has a unique name within this component
  std::string ensure_unique_name ( Ptr subcomp );

  /// writes the underlying component tree to the xml node
  /// @param put_all_content If @c false, options and properties are not put
  /// in the node.
  void write_xml_tree( XML::XmlNode& node, bool put_all_content );

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
  URI m_name;
  /// component current path
  URI m_path;
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

//////////////////////////////////////////////////////////////////////////////

/// Stand-alone function to allocate components of a given type
template < typename T >
boost::shared_ptr<T> allocate_component ( const std::string& name )
{
  typename boost::shared_ptr<T> comp ( new T(name), Deleter<T>() );
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

template < typename T >
inline typename boost::shared_ptr<T> Component::as_ptr()
{
  Component::Ptr me = self();
  cf_assert( is_not_null(me.get()) );
  return boost::dynamic_pointer_cast<T>(me);
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T const> Component::as_ptr() const
{
  return boost::dynamic_pointer_cast<T const>(self());
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T> Component::as_ptr_checked()
{
  typename boost::shared_ptr<T> comp = as_ptr<T>();
  if( is_null(comp) )
    throw CastingFailed( FromHere(), "Cannot cast component " + full_path().string() + " of type " + derived_type_name() + " to " + T::type_name() );
  return comp;
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T const> Component::as_ptr_checked() const
{
  typename boost::shared_ptr<T const> comp = as_ptr<T>();
  if( is_null(comp) )
    throw CastingFailed( FromHere(), "Cannot cast const component " + full_path().string() + " of type " + derived_type_name() + " to const " + T::type_name() );
  return comp;
}

////////////////////////////////////////////////////////////////////////////////

inline Component::ConstPtr Component::as_const() const
{
  return boost::const_pointer_cast<Component const> ( self() );
}

////////////////////////////////////////////////////////////////////////////////

inline Component::Ptr Component::as_non_const() const
{
  return boost::const_pointer_cast<Component> ( self() );
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
