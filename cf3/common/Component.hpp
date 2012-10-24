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

#include "common/AllocatedComponent.hpp"
#include "common/Assertions.hpp"
#include "common/BasicExceptions.hpp"
#include "common/ConnectionManager.hpp"
#include "common/Handle.hpp"
#include "common/SignalHandler.hpp"
#include "common/TaggedObject.hpp"
#include "common/URI.hpp"

namespace boost
{
  // forward declarations for iterator_range
  template<typename T>
  class iterator_range;
  class any;
}

namespace cf3 {
namespace common {

template<class T> class ComponentIterator;
class OptionList;
class PropertyList;

namespace XML { class XmlNode; }

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
  public boost::noncopyable,
  public SignalHandler,
  public ConnectionManager,
  public TaggedObject,
  private boost::enable_shared_from_this<Component>
{

public: // typedef
  /// type of the iterator to Component
  typedef ComponentIterator<Component> iterator;
  /// type of the iterator to constant Component
  typedef ComponentIterator<Component const> const_iterator;

private: // typedef

  /// type for storing the sub components
  typedef std::vector< boost::shared_ptr<Component> > CompStorageT;

  /// Type for storing component lookup-by-name
  typedef std::map<std::string, Uint> CompLookupT;

public: // functions

  /// Get the class name
  static std::string type_name () { return "Component"; }

  /// Contructor
  /// @param name of the component
  Component ( const std::string& name );

  /// Virtual destructor
  virtual ~Component();

  /// @name ITERATORS
  //@{

  /// The begin iterator for a range containing Components
  Component::iterator begin();

  /// The end iterator for a range containing Components
  Component::iterator end();

  /// The begin iterator for a range containing Components (const version)
  Component::const_iterator begin() const;

  /// The end iterator for a range containing Components (const version)
  Component::const_iterator end() const;

  /// The begin iterator for a recursive range containing Components
  Component::iterator recursive_begin();

  /// The end iterator for a recursive range containing Components
  Component::iterator recursive_end();

  /// The begin iterator for a recursive range containing Components (const version)
  Component::const_iterator recursive_begin() const;

  /// The end iterator for a recursive range containing Components (const version)
  Component::const_iterator recursive_end() const;

  //@} END ITERATORS

  /// Access the name of the component
  const std::string& name () const { return m_name; }

  /// Rename the component
  void rename ( const std::string& name );

  /// Construct the full path
  URI uri () const;

  /// Resolves relative elements within a path to complete it.
  /// The path may be relative to this component or absolute.
  /// @param path to a component
  /// @pre path must point to an existing component
  /// @post path statisfies URI::is_complete()
  /// @post path statisfies URI::is_absolute()
  void complete_path ( URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return handle to component or null if it doesn't exist
  /// @warning the return type is non-const!!! ( same reasoning as for parent() )
  Handle<Component> access_component ( const URI& path ) const;

  /// @warning this function is removed, as access_component does not modify the component itself (can access any non-const URI)
  //Handle<Component const> access_component ( const URI& path ) const;

  /// Looks for a component via its path
  /// @param path to the component
  /// @return handle to component that is always valid
  /// @throws InvalidURI in case a component is not found at that path
  Handle<Component> access_component_checked ( const URI& path );
  Handle<Component const> access_component_checked ( const URI& path ) const;

  /// Get a handle to the component
  Handle<Component> handle() { return Handle<Component>(shared_from_this()); }
  Handle<Component const> handle() const { return Handle<Component const>(shared_from_this()); }

  template<typename ComponentT>
  Handle<ComponentT> handle() { return Handle<ComponentT>(shared_from_this()); }
  template<typename ComponentT>
  Handle<ComponentT const> handle() const { return Handle<ComponentT const>(shared_from_this()); }

  /// @returns the handle to the parent component, which can be null if there is no parent
  Handle<Component> parent() const;

  /// @returns the upper-most component in the tree, or self if there is no parent
  Handle<Component const> root() const;
  Handle<Component> root();

  /// Gets the named child component from the list of direct subcomponents.l
  /// @return handle to the component. Null if not found.
  Handle<Component> get_child(const std::string& name);
  Handle<Component const> get_child(const std::string& name) const;

  /// Gets the named child component from the list of direct subcomponents.
  /// @throws ValueNotFound in case a component with given name is not found
  /// @return Always valid handle to the component
  Handle<Component> get_child_checked(const std::string& name);
  Handle<Component const> get_child_checked(const std::string& name) const;

  /// @brief Build a (sub)component of this component using the extended type_name of the component.
  ///
  /// The Library is extracted from the extended type_name, and inside is searched for the builder.
  /// The builder creates the component. The component is then added as a subcomponent.
  /// See @ref using_create_component "here" for a tutorial
  Handle<Component> create_component ( const std::string& name , const std::string& builder );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
  Handle<T> create_component( const std::string& name )
  {
    boost::shared_ptr<T> comp = allocate_component<T>(name);
    add_component( comp );
    return Handle<T>(comp);
  }
  
  /// Create a component using the reduced builder name
  template<typename T>
  Handle<T> create_component(const std::string& name , const std::string& builder );

  /// Create a static "always there" subcomponent
  template < typename T >
  Handle<T> create_static_component ( const std::string& name )
  {
    boost::shared_ptr<T> comp = allocate_component<T>(name);
    add_static_component( comp );
    return Handle<T>(comp);
  }

  /// Add the passed component as a subcomponent
  Component& add_component ( const boost::shared_ptr<Component>& subcomp );

  /// Add a link to the passed component, as child. The name of the link will be the same as the
  /// name of the passed component.
  void add_link(Component& linked_component);

  /// Remove a (sub)component of this component
  boost::shared_ptr<Component> remove_component ( const std::string& name );

  /// Remove a (sub)component of this component
  boost::shared_ptr<Component> remove_component ( Component& subcomp );
  
  /// Remove all sub-components of this component, except for the static ones
  void clear();

  /// Move this component to within another one
  /// @param to_parent will be the new parent of this component
  void move_to ( Component& to_parent );

  /// @returns a string representation of the tree below this component
  /// @param [in] basic_mode  if true, only components marked_basic will be printed
  /// @param [in] depth       defines howmany recursions should maximally be performed
  ///                         (default value depth=0 means full tree)
  /// @param [in] level       recursion parameter, should not be touched
  std::string tree(bool basic_mode=false, Uint depth=0, Uint recursion_level=0) const;

  /// @returns info on this component
  /// @param [in] what   every character of this string represents what to output:
  ///                     c : sub components
  ///                     o : options
  ///                     s : signals
  ///                     p : properties
  ///                     t : tags
  std::string info(const std::string& what = std::string("cospt")) const;

  /// @return Returns the number of children this component has.
  size_t count_children() const;

  /// @return Returns the type name of the subclass, according to
  /// @c cf3::common::TypeInfo
  virtual std::string derived_type_name() const = 0;

  /// @return Returns a reference to the property list
  PropertyList& properties();

  /// @return Returns a constant reference to the property list
  const PropertyList& properties() const;

  /// @return Returns a reference to the options list
  OptionList& options();

  /// @return Returns a constant reference to the options list
  const OptionList& options() const;
  
  /// Reset all options to their default value
  void reset_options();

  /// Configures one property recursevely through this component children,
  /// triggering its actions. If an option has the tag "norecurse" recursion is inhibited
  /// on that option
  /// @param [in] optname  The option name
  /// @param [in] val      The new value assigned to the option
  void configure_option_recursively(const std::string& optname, const boost::any& val);

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
  void signal_list_tree( SignalArgs& args ) const;

  ///  prints tree recursively
  void signal_list_tree_recursive ( SignalArgs& args) const;

  /// lists the properties of this component
  void signal_list_properties ( SignalArgs& args ) const;

  /// lists the properties of this component
  void signal_list_options ( SignalArgs& args ) const;

  ///  prints all options recursive
  void signal_list_options_recursive ( SignalArgs& args ) const;

  /// lists the signals of this component
  void signal_list_signals ( SignalArgs& args ) const;

  ///  prints all signals recursive
  void signal_list_signals_recursive ( SignalArgs& args ) const;

  ///  gets info on this component
  void signal_print_info ( SignalArgs& args ) const;

  ///  signal to print the tree
  void signal_print_tree ( SignalArgs& args ) const;

  ///  signature to signal_print_tree
  void signature_print_tree ( SignalArgs& args ) const;

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
  
  /// Signal to store the timings (if enabled) into properties, i.e. for readout from python or the GUI
  void signal_store_timings( SignalArgs& args );
  
  /// Signal to remove all sub-components
  void signal_clear( SignalArgs& args );
  
  /// Signal to set all options to their default value
  void signal_reset_options( SignalArgs& args );

  //@} END SIGNALS

  /// marks this component as basic.
  Component& mark_basic();

  /// Put all subcomponents in a given vector, optionally recursive
  /// @param [out] vec  A vector of all (recursive) subcomponents
  /// @param [in] recurse If true, recurse through all subcomponents.
  ///             If false, puts only direct children
  template<typename ComponentT>
  void put_components(std::vector< boost::shared_ptr<ComponentT> >& vec, const bool recurse);

  /// Put all subcomponents in a given vector, optionally recursive
  /// @param [out] vec  A vector of all (recursive) subcomponents
  /// @param [in] recurse If true, recurse through all subcomponents.
  ///             If false, puts only direct children
  template<typename ComponentT>
  void put_components(std::vector< boost::shared_ptr<ComponentT const> >& vec, const bool recurse) const;



protected: // functions
  /// Add a static (sub)component of this component
  Component& add_static_component ( const boost::shared_ptr<Component>& subcomp );

private: // helper functions

  /// Modify the parent of this component
  void change_parent(Handle<Component> to_parent);

  /// insures the sub component has a unique name within this component
  std::string ensure_unique_name ( Component& subcomp );

  /// writes the underlying component tree to the xml node
  /// @param node            xml node to write
  /// @param put_all_content If @c false, options and properties are not put
  /// in the node.
  void write_xml_tree( XML::XmlNode& node, bool put_all_content ) const;

  /// Triggered when the "ping" event is raised. Useful to find out what components still exist
  void on_ping_event( SignalArgs& args );

private: // data

  /// component name (stored as path to ensure validity)
  std::string m_name;
  /// storage of the property list (pointer to avoid header include)
  boost::shared_ptr<PropertyList> m_properties;
  /// storage of the option list (pointer to avoid header include)
  boost::shared_ptr<OptionList> m_options;
  /// list of sub-components
  CompStorageT m_components;
  /// lookup of the index of a component
  CompLookupT m_component_lookup;
  /// pointer to parent, naked pointer because of static components
  Component* m_parent;

protected: // functions

  /// raise event that the path has changed
  void raise_tree_updated_event();

  /// Friend declarations allow enable_shared_from_this to be private
  template<class T> friend class boost::enable_shared_from_this;
  template<class T> friend class boost::shared_ptr;
}; // Component


////////////////////////////////////////////////////////////////////////////////////////////


template<typename ComponentT>
inline void Component::put_components(std::vector< boost::shared_ptr< ComponentT > >& vec, const bool recurse)
{
  for(CompStorageT::iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    boost::shared_ptr<ComponentT> p = boost::dynamic_pointer_cast<ComponentT>(*it);
    if(is_not_null(p))
    {
      vec.push_back(p);
    }
    if(recurse)
      (*it)->put_components<ComponentT>(vec, true);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

template<typename ComponentT>
void Component::put_components(std::vector< boost::shared_ptr< const ComponentT > >& vec, const bool recurse) const
{
  for(CompStorageT::const_iterator it=m_components.begin(); it!=m_components.end(); ++it)
  {
    boost::shared_ptr<ComponentT> p = boost::dynamic_pointer_cast<ComponentT>(*it);
    if(is_not_null(p))
    {
      vec.push_back(p);
    }
    if(recurse)
      (*it)->put_components<ComponentT>(vec, true);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

// specialization avoiding the dynamic cast
template<>
inline void Component::put_components<Component>(std::vector< boost::shared_ptr<Component> >& vec, const bool recurse)
{
  if(recurse)
  {
    for(CompStorageT::iterator it=m_components.begin(); it!=m_components.end(); ++it)
    {
      vec.push_back(*it);
      (*it)->put_components<Component>(vec, true);
    }
  }
  else
  {
    vec.insert(vec.end(), m_components.begin(), m_components.end());
  }
}

template<>
inline void Component::put_components<Component>(std::vector<boost::shared_ptr<Component const> >& vec, const bool recurse) const
{
  if(recurse)
  {
    for(CompStorageT::const_iterator it=m_components.begin(); it!=m_components.end(); ++it)
    {
      vec.push_back(*it);
      (*it)->put_components<Component>(vec, true);
    }
  }
  else
  {
    vec.insert(vec.end(), m_components.begin(), m_components.end());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

/// Create a component by providing the name of its builder
/// No factory name is needed, so no factories are used (also no auto-loading of factory).
/// Component is built directly from the builder.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @param provider_name the registry string of the provider
/// @name name to give to the created omponent
boost::shared_ptr<Component> build_component(const std::string& builder_name,
                               const std::string& name);

/// Same as before, but will not throw and return null if something went wrong
boost::shared_ptr<Component> build_component_nothrow(const std::string& builder_name,
                               const std::string& name);


/// Create a component by providing the name of its builder.
/// If factory does not exist, tries to auto-load based on the factory name.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @pre Factory must be contain the builder defined by the name
/// @param [in] provider_name the registry string of the provider
/// @param [in] name name to give to the created component
/// @param [in] factory_type_name name of the factory
boost::shared_ptr<Component> build_component(const std::string& builder_name,
                               const std::string& name,
                               const std::string& factory_type_name);

/// Create a component by providing the name of its builder.
/// If factory does not exist, tries to auto-load based on the factory name.
/// If builder does not exist, tries to auto-load based on the builder name.
/// @pre Factory must be contain the builder defined by the name
/// @param [in] provider_name the registry string of the provider
/// @param [in] name name to give to the created component
/// @param [in] factory_type_name name of the factory
boost::shared_ptr<Component> build_component_reduced(const std::string& builder_name,
                                       const std::string& name,
                                       const std::string& factory_type_name);

/// Create a component of a given abstract type
/// @param provider_name the registry string of the provider of the concrete type
/// @name name to give to the created omponent
template < typename ATYPE >
boost::shared_ptr<ATYPE> build_component_abstract_type(const std::string& builder_name,
                                                  const std::string& name )
{
  boost::shared_ptr<ATYPE> comp = boost::dynamic_pointer_cast<ATYPE>(build_component(builder_name, name, ATYPE::type_name()));

  if ( is_null(comp) )
    throw CastingFailed(FromHere(),
                        "Pointer created by Builder \'" + builder_name + "\'"
                        +" could not be casted to \'" + ATYPE::type_name() + "\' pointer" );

  return comp;
}

/// Create a component of a given abstract type using a reduced builder name
/// @param buider_name the reduced builder name (name without namespace)
/// @name name to give to the created omponent
template < typename ATYPE >
boost::shared_ptr<ATYPE> build_component_abstract_type_reduced(const std::string& builder_name,
                                                          const std::string& name )
{
  boost::shared_ptr<ATYPE> comp = boost::dynamic_pointer_cast<ATYPE>(build_component_reduced(builder_name, name, ATYPE::type_name()));

  if ( is_null(comp) )
    throw CastingFailed(FromHere(),
                        "Pointer created by Builder \'" + builder_name + "\'"
                        +" could not be casted to \'" + ATYPE::type_name() + "\' pointer" );

    return comp;
}

template<typename T>
Handle< T > Component::create_component(const std::string& name, const std::string& builder)
{
  boost::shared_ptr<T> comp;
  if(std::count(builder.begin(), builder.end(), '.') != 0)
  {
    comp = build_component_abstract_type<T>(builder, name);
  }
  else
  {
    comp = build_component_abstract_type_reduced<T>(builder, name);
  }
  add_component( comp );
  return Handle<T>(comp);
}


////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif // cf3_common_Component_hpp
