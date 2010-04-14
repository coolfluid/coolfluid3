#include "Common/Component.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Component::Component ( const CName& name, const CPath& parent_path ) :
    m_name (),
    m_path (parent_path)
{
  if (!CPath::is_valid_element( name ))
    throw InvalidPath(FromHere(), "Component name ["+name+"] is invalid");
  m_name = name;
}

Component::~Component()
{
  /// @todo implement
}

void Component::rename ( const CName& name )
{
  m_name = name; // rename object

  /// @todo inform the tree manager
}

void Component::add_component ( Component * subcomp )
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

void Component::change_path (const CPath &new_path)
{
  m_path = new_path;

  /// @todo inform the tree manager
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
