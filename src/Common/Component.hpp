// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
#include "Common/CPath.hpp"
#include "Common/ConcreteProvider.hpp"
#include "Common/ComponentIterator.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/OptionArray.hpp"
#include "Common/TaggedObject.hpp"

namespace CF {
namespace Common {

  class XmlParams;
  class Option;

////////////////////////////////////////////////////////////////////////////////

/// Base class for defining CF components
/// @author Tiago Quintino
/// @author Willem Deconinck
/// @todo add ownership of (sub) components

class Common_API Component
  :
  public boost::enable_shared_from_this<Component>,
  public ConfigObject,
  public SignalHandler,
  public TaggedObject,
  public boost::noncopyable {

public: // typedef

  /// type for names of components
  typedef std::string CName;
  /// type of this class contruction provider
  typedef Common::ConcreteProvider < Component, NB_ARGS_1 > PROVIDER;
  /// type of first argument of constructor
  typedef const CName& ARG1;
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
  typedef std::map < CName , Component::Ptr > CompStorage_t;

public: // functions

  /// Get the class name
  static std::string type_name () { return "Component"; }

  /// Configuration properties
  static void defineConfigProperties ( Common::PropertyList& props );

  /// Contructor
  /// @param name of the component
  /// @param parent path where this component will be placed
  Component ( const CName& name );

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
  CName name () const { return m_name.string(); }

  /// Rename the component
  void rename ( const CName& name );

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
  Ptr get_child(const CName& name);
  ConstPtr get_child(const CName& name) const;

  /// @returns the named child from the direct subcomponents automatically cast to the specified type
  template < typename T >
      typename T::Ptr get_child_type ( const CName& name );

  /// @returns the named child from the direct subcomponents automatically cast to the specified type
  template < typename T >
      typename T::ConstPtr get_child_type ( const CName& name ) const ;

  /// @returns this component converted to type T shared pointer
  template < typename T >
    typename T::Ptr get_type();

  /// @returns this component converted to type T shared const pointer
  template < typename T >
    typename T::ConstPtr get_type() const;

  /// Modify the parent of this component
  void change_parent ( Component* new_parent );

  /// Create a (sub)component of this component automatically cast to the specified type
  template < typename T >
      typename T::Ptr create_component_type ( const CName& name );

  /// Add a dynamic (sub)component of this component
  Ptr add_component ( Ptr subcomp );

  /// Remove a (sub)component of this component
  Ptr remove_component ( const CName& name );

  /// Move this component to within another one
  /// @param new_parent will be the new parent of this component
  void move_component ( Ptr new_parent );

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
  PropertyList & properties() { return m_property_list; }

  /// @return Returns a constant referent to the property list
  const PropertyList & properties() const { return m_property_list; }

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

protected: // functions

  /// Must be called in constructor of each derived class
 template <typename TYPE> void partial_build_component (TYPE* meself);

 /// Add a static (sub)component of this component
 Ptr add_static_component ( Ptr subcomp );

private: // helper functions

  /// insures the sub component has a unique name within this component
  Component::Ptr ensure_unique_name ( Ptr subcomp );

  /// writes the underlying component tree to the xml node
  void write_xml_tree( XmlNode& node );

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self );

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

  /// Adds an array to XML tree
  /// @param params XmlParams object that manages the tree
  /// @param array Array to add. It must be an OptionArrayT<TYPE>
  template<typename TYPE>
  void add_array_to_xml(XmlParams & params, const std::string & name,
                        boost::shared_ptr<OptionArray> array) const;

  /// Adds an array to XML tree
  /// @param params XmlParams object that manages the tree
  /// @param prop Property to add
  template<typename TYPE>
  void add_prop_to_xml(XmlParams & params, const std::string & name,
                       boost::shared_ptr<Property> prop) const;

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
  boost::weak_ptr<Component> m_root;
  /// pointer to parent, naked pointer because of static components
  Component * m_raw_parent;
  /// is this a link component
  bool m_is_link;

}; // Component

////////////////////////////////////////////////////////////////////////////////

/// Create a (sub)component of a given abstract type specified type
/// @param provider_name the registry string of the provider of the concrete type
/// @name name to give to the created omponent
template < typename ATYPE >
    typename ATYPE::Ptr create_component_abstract_type ( const std::string& provider_name, const Component::CName& name )
{
  Common::SafePtr< typename ATYPE::PROVIDER > prov =
      Common::Factory<ATYPE>::instance().getProvider( provider_name );
  return boost::dynamic_pointer_cast<ATYPE>( prov->create(name) );
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::create_component_type ( const CName& name )
{
  typename T::Ptr new_component ( new T(name), Deleter<T>() );
  return boost::dynamic_pointer_cast<T>( add_component( new_component ) );
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::Ptr Component::get_child_type(const CName& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T>(found->second);
  return boost::dynamic_pointer_cast<T>(Ptr());
}

////////////////////////////////////////////////////////////////////////////////

template < typename T >
inline typename T::ConstPtr Component::get_child_type(const CName& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return boost::dynamic_pointer_cast<T const>(found->second);
  return boost::dynamic_pointer_cast<T const>(ConstPtr());
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
    if(typename ComponentT::Ptr componentPtr = boost::dynamic_pointer_cast<ComponentT>(it->second))
    {
      vec.push_back(componentPtr);
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
    if(boost::shared_ptr<ComponentT const> componentPtr = boost::dynamic_pointer_cast<ComponentT const>(it->second))
    {
      vec.push_back(componentPtr);
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
  return begin<Component>();
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
  return recursive_begin<Component>();
}

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
inline void Component::partial_build_component(TYPE* meself)
{
  addConfigOptionsTo<TYPE>();
  add_tag( TYPE::type_name() );
}

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
    void Component::add_array_to_xml(XmlParams & params, const std::string & name,
                                 boost::shared_ptr<OptionArray> array) const
{
  boost::shared_ptr<OptionArrayT<TYPE> > optArray;
  bool basic = array->has_tag("basic");
  std::string desc = array->description();

  optArray = boost::dynamic_pointer_cast< OptionArrayT<TYPE> >(array);

  params.add_array(name, boost::any_cast< std::vector<TYPE> >(optArray->value()), desc, basic);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
void Component::add_prop_to_xml(XmlParams & params, const std::string & name,
                                 boost::shared_ptr<Property> prop) const
{
  TYPE value = prop->value<TYPE>();

  if(!prop->is_option())
    params.add_property(name, value);
  else
  {
    Option & opt = prop->as_option();
    bool basic = opt.has_tag("basic");
    std::string desc = opt.description();

    if(opt.has_restricted_list())
      params.add_option(name, value, desc, basic, opt.restricted_list());
    else
      params.add_option(name, value, desc, basic);
  }
}

////////////////////////////////////////////////////////////////////////////////

#define BUILD_COMPONENT             \
    partial_build_component(this);  \
    regist_signals(this)

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_Component_hpp
