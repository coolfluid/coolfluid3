#include "Common/CRoot.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<CRoot> CRoot::create ( const CName& name )
  {
    boost::shared_ptr<CRoot> root ( new CRoot(name) );
    root->m_root = root;
    return root;
  }

  CRoot::CRoot ( const CName& name ) : Component ( name )
  {
    m_path = "/";
  }

  CRoot::~CRoot()
  {
  }

  boost::shared_ptr<Component> CRoot::access_component( const CPath& path )
  {
    cf_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_component_storage.find(path.string());
    if ( itr != m_component_storage.end() )
      return itr->second;
    else
      throw InvalidPath(FromHere(), "No component exists with path [" + path.string() + "]");
  }

  void CRoot::define_component_path( const CPath& path , boost::shared_ptr<Component> comp )
  {
    cf_assert ( path.is_complete() );

    // remove the current path of the component, if exists
    CompStorage_t::iterator old = m_component_storage.find( comp->full_path().string() );
    if ( old != m_component_storage.end() )
      m_component_storage.erase(old);

    // set the new path
    CompStorage_t::iterator itr = m_component_storage.find(path.string());
    if ( itr != m_component_storage.end() )
      throw ValueExists(FromHere(), "A component exists with path [" + path.string() + "]");

    m_component_storage[path.string()] = comp;
  }

 ////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
