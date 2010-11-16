// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/Component.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentIterator.hpp"
#include "Common/OptionArray.hpp"
#include "Common/String/Conversion.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

Component::Component ( const CName& name ) :
    m_name (),
    m_path (),
    m_components(),
    m_dynamic_components(),
    m_raw_parent(NULL),
    m_is_link (false)
{
  BUILD_COMPONENT;

  if (!CPath::is_valid_element( name ))
    throw InvalidPath(FromHere(), "Component name ["+name+"] is invalid");

  m_name = name;

  m_property_list.add_property("brief",
                               std::string("No brief description available"));
  m_property_list.add_property("description",
                               std::string("This component has not a long description"));
}

Component::~Component()
{
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::defineConfigProperties ( Common::PropertyList& props )
{
}

/////////////////////////////////////////////////////////////////////////////////////


Component::Ptr Component::get()
{
  return shared_from_this();
}

Component::ConstPtr Component::get() const
{
  return shared_from_this();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::rename ( const CName& name )
{
  std::string new_name = name;
  if ( new_name == m_name.string() ) // skip if name does not change
    return;

  CPath new_full_path = m_path / new_name;

  if( !m_root.expired() )
  {
    // get the root and inform about the change in path
    boost::shared_ptr<CRoot> root =
        boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

    root->change_component_path( new_full_path , shared_from_this() );
  }

  // inform parent of rename
  if (!m_raw_parent)
  {
    Component::Ptr parent = m_raw_parent->self();
    parent->remove_component(m_name.string());
    m_name = new_name;
    Component::Ptr this_component = parent->add_component( get() );
    new_name = this_component->name();
  }

  m_name = new_name;

//  raise_path_changed();

  // loop on children and inform them of change in name
  /// @todo solve this, maybe putting finally uuid's in the comps and using the root to get the path
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::get_parent()
{
  cf_assert( m_raw_parent != NULL );
  return m_raw_parent->self();
}

Component::ConstPtr Component::get_parent() const
{
  cf_assert( m_raw_parent != NULL );
  return m_raw_parent->self();
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::add_component ( Component::Ptr subcomp )
{
  std::string unique_name = ensure_unique_name(subcomp);

  m_components[unique_name] = subcomp;           // add to all component list
  m_dynamic_components[unique_name] = subcomp;   // add to dynamic component list

  raise_path_changed();

	subcomp->change_parent( this );
	subcomp->rename( unique_name );

	return subcomp;
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::add_static_component ( Component::Ptr subcomp )
{
  std::string unique_name = ensure_unique_name(subcomp);

  m_components[unique_name] = subcomp;

	subcomp->change_parent( this );
	subcomp->rename( unique_name );

	return subcomp;
}

/////////////////////////////////////////////////////////////////////////////////////

std::string Component::ensure_unique_name ( Component::Ptr subcomp )
{
	const std::string name = subcomp->name();
	std::string new_name = name;
	boost::regex e(name+"(_[0-9]+)?");
	BOOST_FOREACH(CompStorage_t::value_type subcomp_pair, m_components)
	{
		if (boost::regex_match(subcomp_pair.first,e))
		{
			Uint count = 1;

			new_name = name + "_" + String::to_str(count);

      // make sure constructed name does not exist
      while ( m_components.find(new_name) != m_components.end() )
      {
        ++count;
        new_name = name  + "_" + String::to_str(count);
      }

//      CFwarn << "Component named \'" << subcomp->full_path().string() << "\' already exists. Component renamed to \'" << new_name << "\'" << CFendl;

      break;
    }
  }
  return new_name;
}

/////////////////////////////////////////////////////////////////////////////////////


Component::Ptr Component::remove_component ( const CName& name )
{
  // find the component exists
  Component::CompStorage_t::iterator itr = m_dynamic_components.find(name);

  if ( itr != m_dynamic_components.end() )         // if exists
  {
    Component::Ptr comp = itr->second;             // get the component
    m_dynamic_components.erase(itr);               // remove it from the storage

    // remove fromt the list of all components
    Component::CompStorage_t::iterator citr = m_components.find(name);
    m_components.erase(citr);

    comp->change_parent( NULL );         // set parent to invalid

    raise_path_changed();

    return comp;                                   // return it to client
  }
  else                                             // if does not exist
  {
    throw ValueNotFound(FromHere(), "Dynamic component with name '"
                        + name + "' does not exist in component '"
                        + this->name() + "' with path ["
                        + m_path.string() + "]");
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::complete_path ( CPath& path ) const
{
  using namespace boost::algorithm;

//  CFinfo << "PATH [" << path.string() << "]\n" << CFflush;

  if (!m_raw_parent)
    throw  InvalidPath(FromHere(), "Component \'" + name() + "\' has no parent");

  if (m_root.expired())
    throw  InvalidPath(FromHere(), "Component \'" + name() + "\' has no root");

  boost::shared_ptr<Component> parent = m_raw_parent->self();
  boost::shared_ptr<Component> root   = m_root.lock();

  std::string sp = path.string();

  if ( path.is_relative() ) // transform it to absolute
  {
    if ( starts_with(sp,"/") ) // remove leading "/" if any
      boost::algorithm::replace_first(sp, "/", "" );

    // substitute leading "../" for full_path() of parent
    if (starts_with(sp,".."))
    {
      std::string pfp = parent->full_path().string();
      boost::algorithm::replace_first(sp, "..", pfp);
    }
    else // substitute leading "./" for full_path() of this component
      if (starts_with(sp,"."))
      {
        boost::algorithm::replace_first(sp, ".", full_path().string());
      }
  }

  cf_assert ( CPath(sp).is_absolute() );

  // break path in tokens and loop on them, while concatenaitng to a new path
  boost::char_separator<char> sep("/");
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tok (sp,sep);

  path = "/" ;
  std::string last;
  for(tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el)
  {
    if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

    if ( equals (*el, "..") )              // substitute any "../" for base path
      path = path.base_path();
    else
      path /= *el;
  }

//  CFinfo << "FINAL PATH: [" << path.string() << "]\n" << CFflush;

  cf_assert ( path.is_complete() );
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::get_child(const CName& name)
{
  const CompStorage_t::iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return Ptr();
}

Component::ConstPtr Component::get_child(const CName& name) const
{
  const CompStorage_t::const_iterator found = m_components.find(name);
  if(found != m_components.end())
    return found->second;
  return ConstPtr();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::change_parent ( Component* new_parent )
{
  if( !m_root.expired() )   // get the root and remove the current path
  {
    boost::shared_ptr<CRoot> root =
        boost::dynamic_pointer_cast<CRoot>( m_root.lock() );
    root->remove_component_path(full_path());
  }

  if( new_parent ) // valid ?
  {
    m_path = new_parent->full_path(); // modify the path
    m_root = new_parent->m_root;      // modify the root

    if( !m_root.expired() )   // get the root and set the new path
    {
      boost::shared_ptr<CRoot> root =
          boost::dynamic_pointer_cast<CRoot>( m_root.lock() );
      root->change_component_path( full_path() , shared_from_this() );
    }
  }
  else // new parent is invalid
  {
    m_path = "";
    m_root.reset();
  }

  // modifiy the parent, may be NULL
  m_raw_parent = new_parent;

  // modify the children
  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    c.second->change_parent( this );
  }

  /// @bug event is raised for each child
//  raise_path_changed();
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::move_component ( Component::Ptr new_parent )
{
  Component::Ptr this_ptr = get_parent()->remove_component( this->name() );
  new_parent->add_component( this_ptr );
  raise_path_changed();
}

/////////////////////////////////////////////////////////////////////////////////////

Component::ConstPtr Component::look_component ( const CPath& path ) const
{
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    CPath lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the root
    boost::shared_ptr<CRoot> root = boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

    return root->access_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    std::string sp = path.string();

    // break path in tokens and loop on them, while concatenaitng to a new path
    boost::char_separator<char> sep("/");
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tok (sp,sep);

    Component::ConstPtr look_comp = get() ;
    std::string last;
    for(tokenizer::const_iterator el=tok.begin(); el!=tok.end(); ++el)
    {
      if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

      if ( equals (*el, "..") )              // substitute any "../" for base path
        look_comp = look_comp->get_parent();
      else
      {
        Component::ConstPtr parent = look_comp;
        look_comp = look_comp->get_child(*el);
        if(!look_comp.get())
          throw ValueNotFound (FromHere(), "Component with name " + *el + " was not found in " + parent->full_path().string());
      }
    }
    return look_comp;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::look_component ( const CPath& path )
{
  if (!m_root.expired())  // root is available. This is a faster method.
  {
    CPath lpath = path;

    complete_path(lpath); // ensure the path is complete

    // get the root
    boost::shared_ptr<CRoot> root =
    boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

    return root->access_component(lpath);
  }
  else // we are in the case with no root. Hence the path must be relative
  {
    using namespace boost::algorithm;

    std::string sp = path.string();

    // break path in tokens and loop on them, while concatenaitng to a new path
    boost::char_separator<char> sep("/");
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    tokenizer tok (sp,sep);

    Component::Ptr look_comp = get() ;
    std::string last;
    for(tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el)
    {
      if ( equals (*el, ".") ) continue;     // substitute any "/./" for nothing

      if ( equals (*el, "..") )              // substitute any "../" for base path
        look_comp = look_comp->get_parent();
      else
      {
        Component::Ptr parent = look_comp;
        look_comp = look_comp->get_child(*el);
        if(!look_comp.get())
          throw ValueNotFound (FromHere(), "Component with name " + *el + " was not found in " + parent->full_path().string());
      }
    }
    return look_comp;
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::regist_signals ( Component* self  )
{
  self->regist_signal ( "create_component" , "creates a component", "Create component" )->connect ( boost::bind ( &Component::create_component, self, _1 ) );

  self->regist_signal ( "list_tree" , "lists the component tree inside this component", "List tree" )->connect ( boost::bind ( &Component::list_tree, self, _1 ) );

  self->regist_signal ( "list_properties" , "lists the options of this component", "List properties" )->connect ( boost::bind ( &Component::list_properties, self, _1 ) );

  self->regist_signal ( "list_signals" , "lists the options of this component", "List signals" )->connect ( boost::bind ( &Component::list_signals, self, _1 ) );

  self->regist_signal ( "configure" , "configures this component", "Configure" )->connect ( boost::bind ( &Component::configure, self, _1 ) );

  self->regist_signal ( "rename_component" , "Renames this component", "Rename" )->connect ( boost::bind ( &Component::rename_component, self, _1 ) );


  self->signal("rename_component").m_signature.insert<std::string>("newName", "Component new name");


  self->signal("list_tree").m_is_read_only = true;
}

/////////////////////////////////////////////////////////////////////////////////////


void Component::create_component ( XmlNode& node  )
{
  XmlParams p ( node );

  std::string name  = p.get_option<std::string>("name");
  std::string atype = p.get_option<std::string>("atype");
  std::string ctype = p.get_option<std::string>("ctype");


  SafePtr< FactoryBase > factory =
      Core::instance().getFactoryRegistry()->getFactory(atype);

  SafePtr< ProviderBase > prov = factory->getProviderBase(ctype);

  /// @todo finish implementation of create_component:
  ///      * how to create a Component without specifying the type?
  ///      * how to pass the constructor parameters?

  throw NotImplemented( FromHere(), "" );
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::write_xml_tree( XmlNode& node )
{
  std::string type_name = derived_type_name();

  if(type_name.empty())
    CFerror << "Unknown derived name for " << DEMANGLED_TYPEID(*this)
        << ". Was this class added to the object provider?" << CFendl;
  else
  {
    XmlNode& this_node = *XmlOps::add_node_to(node, "node");
    XmlOps::add_attribute_to( this_node, "name", name() );
    XmlOps::add_attribute_to( this_node, "atype", type_name);
    XmlOps::add_attribute_to( this_node, "mode", has_tag("basic") ? "basic" : "adv");

    if( m_is_link ) // if it is a link, we put the target path as value
      this_node.value( this_node.document()->allocate_string( get()->full_path().string().c_str() ));
    else
    {
      XmlNode& options = *XmlOps::add_node_to( this_node, XmlParams::tag_node_map());

      // add properties and signals
      list_properties(options);
      list_signals(options);

      BOOST_FOREACH( CompStorage_t::value_type c, m_components )
      {
        c.second->write_xml_tree( this_node );
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_tree( XmlNode& xml )
{
  XmlNode& reply = *XmlOps::add_reply_frame( xml );

  XmlOps::add_attribute_to( reply, "sender", full_path().string() );

  write_xml_tree(reply);
}

/////////////////////////////////////////////////////////////////////////////////////

std::string Component::tree(Uint level) const
{
  std::string tree;
  for (Uint i=0; i<level; i++)
    tree += "  ";
  tree += name() + "\n";

  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    tree += c.second->tree(level+1);
  }
  return tree;
}

/////////////////////////////////////////////////////////////////////////////////////

size_t Component::get_child_count() const
{
  return m_components.size();
}

/////////////////////////////////////////////////////////////////////////////////////

/// Adds an array to XML tree
/// @param params XmlParams object that manages the tree
/// @param array Array to add. It must be an OptionArrayT<TYPE>
template<typename TYPE>
void add_array_to_xml(XmlParams & params, const std::string & name,
                      boost::shared_ptr<OptionArray> array)
{
  boost::shared_ptr<OptionArrayT<TYPE> > optArray;
  bool basic = array->has_tag("basic");
  std::string desc = array->description();

  optArray = boost::dynamic_pointer_cast< OptionArrayT<TYPE> >(array);

  params.add_array(name, boost::any_cast< std::vector<TYPE> >(optArray->value()), desc, basic);
}

/// Adds an array to XML tree
/// @param params XmlParams object that manages the tree
/// @param prop Property to add
template<typename TYPE>
void add_prop_to_xml(XmlParams & params, const std::string & name,
                     boost::shared_ptr<Property> prop)
{
  TYPE value = prop->value<TYPE>();

  if(!prop->is_option())
    params.add_property(name, value);
  else
  {
    Option & opt = prop->as_option();
    bool basic = opt.has_tag("basic");
    std::string desc = opt.description();
    XmlNode * node;

    if(opt.has_restricted_list())
      node = params.add_option(name, value, desc, basic, opt.restricted_list());
    else
      node = params.add_option(name, value, desc, basic);

    if(std::strcmp(opt.tag(), "uri") == 0)
    {
      std::vector<std::string> prots = static_cast<OptionURI*>(&opt)->supported_protocols();
      std::vector<std::string>::iterator it = prots.begin();

      for( ; it != prots.end() ; it++)
        XmlOps::add_attribute_to(*node, XmlParams::tag_attr_protocol(), *it);
    }
  }
}

void Component::list_properties( XmlNode& node )
{
  PropertyList::PropertyStorage_t::iterator it = m_property_list.m_properties.begin();

  XmlParams p(*node.parent());

  for( ; it != m_property_list.m_properties.end() ; it++)
  {
    std::string name = it->first;
    Property::Ptr prop = it->second;

    if(std::strcmp(prop->tag(), "array") != 0)
    {
      std::string type = prop->type();

      if(type == "string")
        add_prop_to_xml<std::string>(p, name, prop);
      else if(type == "bool")
        add_prop_to_xml<bool>(p, name, prop);
      else if(type == "integer")
        add_prop_to_xml<int>(p, name, prop);
      else if(type == "unsigned")
        add_prop_to_xml<CF::Uint>(p, name, prop);
      else if(type == "real" || type == "double")
        add_prop_to_xml<CF::Real>(p, name, prop);
      else if(type == "uri")
        add_prop_to_xml<URI>(p, name, prop);
      else if(type == "file")
        add_prop_to_xml<boost::filesystem::path>(p, name, prop);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage \"") + type + "\" type.");
    }
    else
    {
      boost::shared_ptr<OptionArray> optArray;

      optArray = boost::dynamic_pointer_cast<OptionArray>(prop);

      const char * elem_type = optArray->elem_type();

      if(strcmp(elem_type, "string") == 0)
        add_array_to_xml< std::string >(p, name, optArray);
      else if(strcmp(elem_type, "bool") == 0)
        add_array_to_xml< bool >(p, name, optArray);
      else if(strcmp(elem_type, "integer") == 0)
        add_array_to_xml< int >(p, name, optArray);
      else if(strcmp(elem_type, "unsigned") == 0)
        add_array_to_xml< CF::Uint >(p, name, optArray);
      else if(strcmp(elem_type, "real") == 0 || strcmp(elem_type, "double") == 0)
        add_array_to_xml< CF::Real >(p, name, optArray);
      else if(strcmp(elem_type, "file") == 0)
        add_array_to_xml< boost::filesystem::path >(p, name, optArray);
      else
        throw ShouldNotBeHere(FromHere(),
             std::string("Don't know how the manage OptionArrayT<") +
                  elem_type + ">.");
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::list_signals( XmlNode& node )
{
  sigmap_t::iterator it = m_signals.begin();

  XmlNode & value_node = *XmlOps::add_node_to(node, "value");

  XmlOps::add_attribute_to(value_node, XmlParams::tag_attr_key(), XmlParams::tag_key_signals());

  for( ; it != m_signals.end() ; it++)
  {
    XmlNode & map = *XmlParams::add_map_to(value_node, it->first, it->second.m_description);
    XmlOps::add_attribute_to(map, "name", it->second.m_readable_name);
    it->second.m_signature.put_signature(map);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::rename_component ( XmlNode& xml )
{
  XmlParams p(xml);

  std::string new_name = p.get_option<std::string>("newName");

  rename(new_name);
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::raise_path_changed ()
{
  raise_event("tree_updated");
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::raise_event ( const std::string & name )
{
  if( !m_root.expired() )
  {
    boost::shared_ptr<CRoot> root =
        boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

    root->raise_new_event(name, full_path());
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::mark_basic()
{
  add_tag("basic");
}

} // Common
} // CF
