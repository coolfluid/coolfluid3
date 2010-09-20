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
    //m_properties(),
    m_is_link (false)
{
  BUILD_COMPONENT;

  if (!CPath::is_valid_element( name ))
    throw InvalidPath(FromHere(), "Component name ["+name+"] is invalid");

  m_name = name;

  m_property_list.add_property("brief", std::string("No brief description available"));
  m_property_list.add_property("description", std::string("This component has not a long description"));
}

/////////////////////////////////////////////////////////////////////////////////////

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


void Component::rename ( const CName& name , AddOption add_option)
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
  if (!m_parent.expired() )
  {
    Component::Ptr parent = get_parent();
    Component::Ptr removed = parent->remove_component(m_name.string());
    m_name = new_name;
    Component::Ptr this_component = parent->add_component(get(),add_option);
    new_name = this_component->name();
  }

  m_name = new_name;
}

/////////////////////////////////////////////////////////////////////////////////////

Component::Ptr Component::add_component ( Component::Ptr subcomp, AddOption add_option )
{
  switch (add_option)
  {
    case THROW:
      if (m_components.find(subcomp->name()) != m_components.end() )
      {
        throw ValueExists(FromHere(), "Component with name '"
                          + subcomp->name() + "' already exists in component '"
                          + name() + "' with path ["
                          + m_path.string() + "]");
      }
      break;
    case NUMBER:
      // check that no other component with such name exists
    {
      const std::string name = subcomp->name();
      boost::regex e(name+"(_[0-9]+)?");
      BOOST_FOREACH(CompStorage_t::value_type subcomp_pair, m_components)
      {
        if (boost::regex_match(subcomp_pair.first,e))
        {
          CFinfo << "++++ match found: " << name << " ~~ " << subcomp_pair.first << CFendl;
          // A subcomponent with this name already exists.
          // Instead of throwing, append a number to its name.
          if (m_components.find(name) != m_components.end() )
            m_components[name]->rename(name+"_0");

          Uint count = 1;
          //count howmany times the name "name(_[0-9]+)?" occurs (REGEX)
          while (m_components.find(name+"_"+String::to_str(count)) != m_components.end())
            count++;

          std::string new_name = name+"_"+String::to_str(count);

          CFwarn << "A component \"" << subcomp->full_path().string() << "\" already existed. New component added with name \"" << new_name << "\"" << CFendl;
          subcomp->rename(new_name);
          break;
        }
      }
      break;
    }
    default:
      throw ValueNotFound (FromHere(), "No such option exists for the function add_component");
  }

  m_components[subcomp->name()] = subcomp;

  subcomp->change_parent( shared_from_this() );

  return subcomp;
}

/////////////////////////////////////////////////////////////////////////////////////


Component::Ptr Component::remove_component ( const CName& name )
{
  // find the component exists
  Component::CompStorage_t::iterator itr = m_components.find(name);

  if ( itr != m_components.end() )         // if exists
  {
    Component::Ptr comp = itr->second;     // get the component
    m_components.erase(itr);                // remove it from the storage
    comp->change_parent(Component::Ptr());  // set parent to invalid
    return comp;                             // return it to client to do somethinng typically delete it
  }
  else                                        // if does not exist
  {
    throw ValueNotFound(FromHere(), "Component with name '"
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

  if (m_parent.expired())
    throw  InvalidPath(FromHere(), "Component \'" + name() + "\' has no parent");

  if (m_root.expired())
    throw  InvalidPath(FromHere(), "Component \'" + name() + "\' has no root");

  boost::shared_ptr<Component> parent = m_parent.lock();
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

void Component::change_parent ( Component::Ptr new_parent )
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

  // modifiy the parent
  // may be invalid
  m_parent = new_parent;

  // modify the children
  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    c.second->change_parent( shared_from_this() );
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::move_component ( Component::Ptr new_parent )
{
  Component::Ptr this_ptr = m_parent.lock()->remove_component( this->name() );
  new_parent->add_component( this_ptr );
}

/////////////////////////////////////////////////////////////////////////////////////

Component::ConstPtr Component::look_component ( const CPath& path ) const
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
  self->regist_signal ( "create_component" , "creates a component" )->connect ( boost::bind ( &Component::create_component, self, _1 ) );

  self->regist_signal ( "list_tree" , "lists the component tree inside this component" )->connect ( boost::bind ( &Component::list_tree, self, _1 ) );

  self->regist_signal ( "list_properties" , "lists the options of this component" )->connect ( boost::bind ( &Component::list_properties, self, _1 ) );

  self->regist_signal ( "configure" , "configures this component" )->connect ( boost::bind ( &Component::configure, self, _1 ) );

}

/////////////////////////////////////////////////////////////////////////////////////


void Component::create_component ( XmlNode& node  )
{
  XmlParams p ( node );

  std::string name  = p.get_option<std::string>("name");
  std::string atype = p.get_option<std::string>("atype");
  std::string ctype = p.get_option<std::string>("ctype");


  SafePtr< FactoryBase > factory =
      CoreEnv::instance().getFactoryRegistry()->getFactory(atype);

  SafePtr< ProviderBase > prov = factory->getProviderBase(ctype);

  /// @todo finish implementation of create_component:
  ///      * how to create a Component without specifying the type?
  ///      * how to pass the constructor parameters?

  throw NotImplemented( FromHere(), "" );
}

/////////////////////////////////////////////////////////////////////////////////////

void Component::write_xml_tree( XmlNode& node )
{
  if(derived_type_name().empty())
    CFerror << "Unknown derived name for " << DEMANGLED_TYPEID(*this)
        << ". Was this class added to the object provider?" << CFendl;
  else
  {
    XmlNode& this_node = *XmlOps::add_node_to(node, derived_type_name());
    XmlOps::add_attribute_to( this_node, "name", name() );

    if( m_is_link ) // if it is a link, we put the target path as value
      this_node.value( this_node.document()->allocate_string( get()->full_path().string().c_str() ));
    else
    {
      XmlNode& options = *XmlOps::add_node_to( this_node, XmlParams::tag_node_valuemap());

      // add properties
      list_properties(options);

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

} // Common
} // CF
