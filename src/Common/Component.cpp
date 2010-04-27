#include <boost/cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "Common/Component.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Component::Component ( const CName& name ) :
    m_name (),
    m_path (),
    m_is_link (false)
{
  if (!CPath::is_valid_element( name ))
    throw InvalidPath(FromHere(), "Component name ["+name+"] is invalid");
  m_name = name;
}

////////////////////////////////////////////////////////////////////////////////

Component::~Component()
{
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////////

Common::SafePtr<Component> Component::get ()
{
  return this;
}

////////////////////////////////////////////////////////////////////////////////

void Component::rename ( const CName& name )
{
  if ( name == m_name.string() ) // skip if name does not change
    return;

  CPath new_full_path = m_path / name;

  if( !m_root.expired() )
  {
    // get the root and inform about the change in path
    boost::shared_ptr<CRoot> root =
        boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

    root->define_component_path( new_full_path , shared_from_this() );
  }

  // rename object make be after modificatio of path in root
  // else root would not find the previous object
  m_name = name;
}

////////////////////////////////////////////////////////////////////////////////

void Component::add_component ( boost::shared_ptr<Component> subcomp )
{
  // check that no other component with such name exists
  if (m_components.find(subcomp->name()) != m_components.end() )
    throw ValueExists(FromHere(), "Component with name '"
                      + subcomp->name() + "' already exists in component '"
                      + name() + "' with path ["
                      + m_path.string() + "]");

  m_components[subcomp->name()] = subcomp;

  subcomp->change_parent( shared_from_this() );
}

////////////////////////////////////////////////////////////////////////////////

SafePtr<Component> Component::get_component ( const CName& name )
{
  return m_components[name].get();
}

////////////////////////////////////////////////////////////////////////////////

void Component::list_options (XMLNode xml)
{
  /// @todo implement this
}

////////////////////////////////////////////////////////////////////////////////

void Component::complete_path ( CPath& path )
{
  using namespace boost::algorithm;

//  CFinfo << "PATH [" << path.string() << "]\n" << CFendl;

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

//  CFinfo << "FINAL PATH: [" << path.string() << "]\n" << CFendl;

  cf_assert ( path.is_complete() );
}

////////////////////////////////////////////////////////////////////////////////

void Component::change_parent (boost::shared_ptr<Component> new_parent)
{
	if( new_parent ) // valid ?
	{
		m_parent = new_parent;   // modify the parent
    
		m_path = new_parent->full_path(); // modify the path
    m_root = new_parent->m_root;      // modify the root
    
    if( !m_root.expired() )
    {
      // get the root and inform about the change in path
      boost::shared_ptr<CRoot> root =
          boost::dynamic_pointer_cast<CRoot>( m_root.lock() );
      root->define_component_path( full_path() , shared_from_this() );
    }

    // modify the children
    BOOST_FOREACH( CompStorage_t::value_type c, m_components )
    {
      c.second->change_parent( shared_from_this() );
    }
  }
}

boost::shared_ptr<Component> Component::look_component ( const CPath& path )
{
  cf_assert ( ! m_root.expired() );

  CPath lpath = path;

  complete_path(lpath); // ensure the path is complete

  // get the root
  boost::shared_ptr<CRoot> root =
      boost::dynamic_pointer_cast<CRoot>( m_root.lock() );

  return root->access_component(lpath);
}

////////////////////////////////////////////////////////////////////////////////

void Component::xml_tree(XMLNode parent)
{
  XMLNode this_node = parent.addChild( name().c_str() );

  BOOST_FOREACH( CompStorage_t::value_type c, m_components )
  {
    c.second->xml_tree( this_node );
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
