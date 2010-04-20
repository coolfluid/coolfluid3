#include <boost/cast.hpp>
#include <boost/foreach.hpp>

#include "Common/Component.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Component::Component ( const CName& name, const CPath& parent_path ) :
    m_name (),
    m_path (parent_path),
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
  m_name = name; // rename object

  /// @todo inform the tree root
  if( !m_root.expired() )
  {
//    boost::polymorphic_cast<Root>(m_root)->renameNode();
  }
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

  subcomp->change_path( full_path() );
}

////////////////////////////////////////////////////////////////////////////////

void Component::change_path (const CPath &new_path)
{
  m_path = new_path;

  /// @todo inform the tree manager
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
