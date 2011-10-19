// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Component.hpp
/// @brief Holds the Component class, as well as the ComponentIterator class
///        plus some functions related to component creation

#ifndef cf3_common_Component_hpp
#define cf3_common_Component_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/enable_shared_from_this.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>

#include "Common/AllocatedComponent.hpp"
#include "Common/Assertions.hpp"
#include "Common/PropertyList.hpp"
#include "Common/OptionList.hpp"
#include "Common/SignalHandler.hpp"
#include "Common/ConnectionManager.hpp"
#include "Common/URI.hpp"

namespace cf3 {
namespace common {

  class CRoot;

  template<class T> class ComponentIterator;

////////////////////////////////////////////////////////////////////////////////////////////

  /// @brief Stand-alone function to allocate components of a given type
  ///
  /// A shared pointer must be returned, as this creates the very first
  /// instance of the shared_pointer.
  /// This is the most low-level component creation function
  /// @param [in] name The name to give to the component to allocate
  /// @return The component as a boost::shared_ptr
  template < typename T >
  boost::shared_ptr<T> allocate_component ( const std::string& name )
  {
    typedef typename SelectComponentWrapper<T>::type AllocatedComponentT;
    typename boost::shared_ptr<T> comp ( new AllocatedComponentT(name), Deleter<AllocatedComponentT>() );
    return comp ;
  }

////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Base class for defining CF components
///
/// The Component class is the base principle of COOLFluiD.
/// Components can hold other components, much like the filesystem of
/// your OS. See @link introducing_components this page @endlink for
/// a more complete understanding.
///
/// Components:
///  - have dynamic variables (properties).
///  - have configuration options. This are special properties that can
///    trigger a function
///  - have dynamic functions (signals). Signal arguments are in XML format,
///    and thus almost limitless.
///
/// See @link using_components this page @endlink for a tutorial how to work
/// with components.
/// @author Tiago Quintino
/// @author Willem Deconinck
class Common_API Component :
  public boost::enable_shared_from_this<Component>,
  public boost::noncopyable,
  public SignalHandler,
  public ConnectionManager,
  public TaggedObject  {

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
  Component ( const std::string& name );

  /// Virtual destructor
  virtual ~Component();

  /// Get the component through the links to the actual components
  virtual Component::Ptr  follow();
  /// Get the component through the links to the actual components
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

  /// Construct the full path
  URI uri () const { return m_path / m_name; }

  /// Resolves relative elements within a path to complete it.
  /// The path may be relative to this component or absolute.
  /// This is strictly a path operation so the path may not actually point anywhere
  /// @param path to a component
  /// @post path statisfies URI::is_complete()
  /// @post path statisfies URI::is_absolute()
  void complete_path ( URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return reference to component
  Component& access_component ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component or null if no component was found
  ConstPtr access_component_ptr ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return constant Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  ConstPtr access_component_ptr_checked ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component, or null if none was found
  Ptr access_component_ptr ( const URI& path );

  /// Looks for a component via its path
  /// @param path to the component
  /// @return Ptr to component
  /// @throws InvalidURI in case a component is not found at that path
  /// @post returns always valid pointer
  Ptr access_component_ptr_checked ( const URI& path );

  /// @returns true if the component has a parent
  bool has_parent() const;

  /// @returns the pointer to parent component
  /// @pre parent pointer is valid
  /// @post returns always valid pointer
  Component& parent() const;

  /// Gets the named child component from the list of direct subcomponents.
  /// @return reference to the component
  Component& get_child(const std::string& name) const;

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

  /// Gets the named child component from the list of direct subcomponents.
  /// @throws ValueNotFound in case a component with given name is not found
  /// @post pointer is never null
  /// @return shared pointer to the component
  ConstPtr get_child_ptr_checked(const std::string& name) const;

  /// returns an iterator range with the children of this component
  boost::iterator_range<iterator> children();

  /// returns an iterator range with the children of this component
  boost::iterator_range<const_iterator> children() const;

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

  /// Cast the component as a ConstPtr
  /// @return a ConstPtr
  ConstPtr as_const() const;

  /// Cast a const object to a non-const Ptr (to be used with extreme care)
  /// @return a Ptr
  Ptr as_non_const() const;

  /// @brief Build a (sub)component of this component using the extended type_name of the component.
  ///
  /// The Library is extracted from the extended type_name, and inside is searched for the builder.
  /// The builder creates the component. The component is then added as a subcomponent.
  /// See @ref using_create_component "here" for a tutorial
  Component& create_component ( const std::string& name , const std::string& builder );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
    typename T::Ptr create_component_ptr ( const std::string& name );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
    T& create_component ( const std::string& name );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
    typename T::Ptr create_static_component_ptr ( const std::string& name );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
    T& create_static_component ( const std::string& name );

  /// Add a dynamic (sub)component of this component
  Component& add_component ( Ptr subcomp );

  /// Add a dynamic (sub)component of this component
  Component& add_component ( Component& subcomp );

  /// Remove a (sub)component of this component
  Ptr remove_component ( const std::string& name );

  /// Remove a (sub)component of this component
  Ptr remove_component ( Component& subcomp );

  /// Move this component to within another one
  /// @param to_parent will be the new parent of this component
  void move_to ( Component& to_parent );

  /// @returns a string representation of the tree below this component
  std::string tree(Uint level=0) const;

  /// @return Returns the number of children this component has.
  size_t count_children() const;

  /// @return Returns the type name of the subclass, according to
  /// @c CF::Common::TypeInfo
  virtual std::string derived_type_name() const = 0;

  /// @return Returns a reference to the property list
  PropertyList& properties() { return m_properties; }

  /// @return Returns a constant referent to the property list
  const PropertyList& properties() const { return m_properties; }

  /// @return Returns a reference to the property with a provided name
  const boost::any& property(const std::string& optname ) const;

  /// access to the property
  boost::any& property(const std::string& optname );

  /// @return Returns a reference to the option with a provided name
  Option& option(const std::string& optname );

  /// access to the property
  const Option& option(const std::string& optname ) const;

  /// @return Returns a reference to the property list
  OptionList& options() { return m_options; }

  /// @return Returns a constant referent to the property list
  const OptionList& options() const { return m_options; }

  /// Configure one property, and trigger its actions
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  Component& configure_property(const std::string& optname, const boost::any& val);

  /// Configure one option, and trigger its actions
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  Component& configure_option(const std::string& optname, const boost::any& val);

  /// Configures one property recursevely through this component children,
  /// triggering its actions. If an option has the tag "norecurse" recursion is inhibited
  /// on that option
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  void configure_option_recursively(const std::string& optname, const boost::any& val);

  /// Configures all the options on this class from a list of strings.
  /// Each string provides the configuration of one property following the
  /// format var_name:type=value var_name:array[type]=val1,val2
  void configure (const std::vector<std::string>& args);

  /// Creates or modifies existing properties using the CF human readable language
  /// For single variables  --> var_name:type=value @n
  /// For arrays            --> var_name:array[type]=val1,val2
  void change_property(const std::string args);

  /// @name SIGNALS
  //@{

  /// configures all the options on this class
  void signal_configure ( SignalArgs& args );

  /// creates a component from this component
  void signal_create_component ( SignalArgs& args );

  /// deletes a component from this component
  void signal_delete_component ( SignalArgs& args );

  /// moves a component from this component to another
  void signal_move_component ( SignalArgs& args );

  /// lists the sub components and puts them on the xml_tree
  void signal_list_tree( SignalArgs& args );

  /// lists the properties of this component
  void signal_list_properties ( SignalArgs& args );

  /// lists the properties of this component
  void signal_list_options ( SignalArgs& args );

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
  /// @param args The frame under which signature is added.
  void signature_create_component( SignalArgs& args );

  /// Defines the signature of "rename_component" signal.
  /// @param args The frame under which signature is added.
  void signature_rename_component( SignalArgs& args );

  /// Defines the signature of "move_component" signal.
  /// @param args The frame under which signature is added.
  void signature_move_component( SignalArgs& args );

  //@} END SIGNALS

  /// marks this component as basic.
  Component& mark_basic();

protected: // functions

  /// Add a static (sub)component of this component
  Component& add_static_component ( Ptr subcomp );

  /// Add a static (sub)component of this component
  Component& add_static_component ( Component& subcomp );

private: // helper functions

  /// Modify the parent of this component
  void change_parent ( Component* to_parent );

  /// insures the sub component has a unique name within this component
  std::string ensure_unique_name ( Component& subcomp );

  /// writes the underlying component tree to the xml node
  /// @param node            xml node to write
  /// @param put_all_content If @c false, options and properties are not put
  /// in the node.
  void write_xml_tree( XML::XmlNode& node, bool put_all_content );

  /// Put all subcomponents in a given vector, optionally recursive
  /// @param [out] vec  A vector of all (recursive) subcomponents
  /// @param [in] recurse If true, recurse through all subcomponents.
  ///             If false, puts only direct children
  template<typename ComponentT>
  void put_components(std::vector<typename ComponentT::Ptr>& vec, const bool recurse);

  /// Put all subcomponents in a given vector, optionally recursive
  /// @param [out] vec  A vector of all (recursive) subcomponents
  /// @param [in] recurse If true, recurse through all subcomponents.
  ///             If false, puts only direct children
  template<typename ComponentT>
  void put_components(std::vector<boost::shared_ptr<ComponentT const> >& vec, const bool recurse) const;

  /// Returns an iterator
  /// @param [in] begin If true, the begin iterator is returned, otherwise end
  /// @param [out] recursive If true, the iterator recurses over all components below this
  template<typename ComponentT>
  ComponentIterator<ComponentT> make_iterator(const bool begin, const bool recursive);

  /// Returns an iterator
  /// @param [in] begin If true, the begin iterator is returned, otherwise end
  /// @param [out] recursive If true, the iterator recurses over all components below this
  template<typename ComponentT>
  ComponentIterator<ComponentT const> make_iterator(const bool begin, const bool recursive) const;

protected: // data

  /// component name (stored as path to ensure validity)
  URI m_name;
  /// component current path
  URI m_path;
  /// storage of the property list
  PropertyList m_properties;
  /// storage of the option list
  OptionList m_options;
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

protected: // functions

  /// raise event that the path has changed
  void raise_path_changed();
  /// raise event an event with a given name
  void raise_event(const std::string & name );

}; // Component

////////////////////////////////////////////////////////////////////////////////////////////

/// @brief %ComponentIterator class, can linearize a complete tree of components
///
/// The ComponentIterator class is the type for
/// - Component::iterator
/// - Component::const_iterator
///
/// - Using Component::begin() and Component::end() iterates on only 1 deeper level
/// - Using Component::recursive_begin() and Component::recursive_end() iterates
/// on all deeper levels recursively. Iterating will then linearize the tree.

template<class T>
class ComponentIterator :
    public boost::iterator_facade<ComponentIterator<T>,  // iterator
                                  T,                     // Value
                                  boost::bidirectional_traversal_tag, // search direction
                                  T&                     // return type of dereference
                                 >
{
  typedef boost::iterator_facade<ComponentIterator<T>,
                                 T,
                                 boost::random_access_traversal_tag,
                                 T&> BaseT;
public:

  typedef typename BaseT::difference_type difference_type;

  /// Construct an iterator over the given set of components.
  /// If endIterator is true, the iterator is intialized
  /// at the end of the range, otherwise at the beginning.
  explicit ComponentIterator(const std::vector<boost::shared_ptr<T> >& vec,
                             const Uint startPosition)
          : m_vec(vec), m_position(startPosition) {}

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

  /// dereferencing
  T& dereference() const { return *m_vec[m_position]; }
  /// Get a shared pointer to the referenced object
  boost::shared_ptr<T> get() const { return m_vec[m_position]; }
  /// Compatibility with boost filtered_iterator interface,
  /// so base() can be used transparently on all ranges
  ComponentIterator<T>& base() { return *this; }
  /// Compatibility with boost filtered_iterator interface
  const ComponentIterator<T>& base() const { return *this; }

private:
  std::vector<boost::shared_ptr<T> > m_vec;
  Uint m_position;
};

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_component_ptr ( const std::string& name )
{
  typename T::Ptr comp = allocate_component<T>(name);
  add_component( comp );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline T& Component::create_component ( const std::string& name )
{
  return *create_component_ptr<T>(name);
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_static_component_ptr ( const std::string& name )
{
  typename T::Ptr comp = allocate_component<T>(name);
  add_static_component( comp );
  return comp ;
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline T& Component::create_static_component ( const std::string& name )
{
  return *create_static_component_ptr<T>(name);
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T> Component::as_ptr()
{
  Component::Ptr me = self();
  cf_assert( is_not_null(me.get()) );
  return boost::dynamic_pointer_cast<T>(me);
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T const> Component::as_ptr() const
{
  return boost::dynamic_pointer_cast<T const>(self());
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T> Component::as_ptr_checked()
{
  typename boost::shared_ptr<T> comp = as_ptr<T>();
  if( is_null(comp) )
    throw CastingFailed( FromHere(), "Cannot cast component " + uri().string() + " of type " + derived_type_name() + " to " + T::type_name() );
  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename boost::shared_ptr<T const> Component::as_ptr_checked() const
{
  typename boost::shared_ptr<T const> comp = as_ptr<T>();
  if( is_null(comp) )
    throw CastingFailed( FromHere(), "Cannot cast const component " + uri().string() + " of type " + derived_type_name() + " to const " + T::type_name() );
  return comp;
}

////////////////////////////////////////////////////////////////////////////////////////////

inline Component::ConstPtr Component::as_const() const
{
  return boost::const_pointer_cast<Component const> ( self() );
}

////////////////////////////////////////////////////////////////////////////////////////////

inline Component::Ptr Component::as_non_const() const
{
  return boost::const_pointer_cast<Component> ( self() );
}

////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::begin()
{
  return make_iterator<ComponentT>(true, false);
}

inline Component::iterator Component::begin()
{
  return begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::end()
{
  return make_iterator<ComponentT>(false, false);
}

inline Component::iterator Component::end()
{
  return end<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::begin() const
{
  return make_iterator<ComponentT>(true, false);
}

inline Component::const_iterator Component::begin() const
{
  return begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::end() const
{
  return make_iterator<ComponentT>(false, false);
}

inline Component::const_iterator Component::end() const
{
  return end<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::recursive_begin()
{
  return make_iterator<ComponentT>(true, true);
}

inline Component::iterator Component::recursive_begin()
{
  return recursive_begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT> Component::recursive_end()
{
  return make_iterator<ComponentT>(false, true);
}

inline Component::iterator Component::recursive_end()
{
  return recursive_end<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::recursive_begin() const
{
  return make_iterator<ComponentT>(true, true);
}

inline Component::const_iterator Component::recursive_begin() const
{
  return recursive_begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
inline ComponentIterator<ComponentT const> Component::recursive_end() const
{
  return make_iterator<ComponentT>(false, true);
}

inline Component::const_iterator Component::recursive_end() const
{
  return recursive_end<Component>();
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Create a component by providing the name of its builder
/// No factory name is needed, so no factories are used (also no auto-loading of factory).
/// Component is built directly from the builder.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @param provider_name the registry string of the provider
/// @name name to give to the created omponent
Component::Ptr build_component(const std::string& builder_name,
                               const std::string& name);


/// Create a component by providing the name of its builder.
/// If factory does not exist, tries to auto-load based on the factory name.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @pre Factory must be contain the builder defined by the name
/// @param [in] provider_name the registry string of the provider
/// @param [in] name name to give to the created component
/// @param [in] factory_type_name name of the factory
Component::Ptr build_component(const std::string& builder_name,
                               const std::string& name,
                               const std::string& factory_type_name);

/// Create a component by providing the name of its builder.
/// If factory does not exist, tries to auto-load based on the factory name.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @pre Factory must be contain the builder defined by the name
/// @param [in] provider_name the registry string of the provider
/// @param [in] name name to give to the created component
/// @param [in] factory_type_name name of the factory
Component::Ptr build_component_reduced(const std::string& builder_name,
                                       const std::string& name,
                                       const std::string& factory_type_name);

/// Create a component of a given abstract type
/// @param provider_name the registry string of the provider of the concrete type
/// @name name to give to the created omponent
template < typename ATYPE >
typename ATYPE::Ptr build_component_abstract_type(const std::string& builder_name,
                                                  const std::string& name )
{
  // create the component

  Component::Ptr comp = build_component(builder_name, name, ATYPE::type_name());

  // cast the component

  typename ATYPE::Ptr ccomp = comp->as_ptr_checked<ATYPE>();
  if ( is_null(ccomp) )
    throw CastingFailed(FromHere(),
                        "Pointer created by CBuilder \'" + builder_name + "\'"
                        +" could not be casted to \'" + ATYPE::type_name() + "\' pointer" );

  return ccomp;
}

/// Create a component of a given abstract type using a reduced builder name
/// @param buider_name the reduced builder name (name without namespace)
/// @name name to give to the created omponent
template < typename ATYPE >
typename ATYPE::Ptr build_component_abstract_type_reduced(const std::string& builder_name,
                                                          const std::string& name )
{
  // create the component

  Component::Ptr comp = build_component_reduced(builder_name, name, ATYPE::type_name());

  // cast the component

  typename ATYPE::Ptr ccomp = comp->as_ptr<ATYPE>();
  if ( is_null(ccomp) )
    throw CastingFailed(FromHere(),
                        "Pointer created by CBuilder \'" + builder_name + "\'"
                        +" could not be casted to \'" + ATYPE::type_name() + "\' pointer" );

  return ccomp;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif // CF3_common_Component_hpp
