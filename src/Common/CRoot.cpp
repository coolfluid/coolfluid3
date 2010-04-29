#include "Common/CRoot.hpp"
#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<CRoot> CRoot::create ( const CName& name )
  {
    boost::shared_ptr<CRoot> root ( new CRoot(name) );
    root->m_root = root;
    root->m_parent = root;

    root->m_toc[root->full_path().string()] = root; // put himself in the database

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

    CompStorage_t::iterator itr = m_toc.find(path.string());
    if ( itr != m_toc.end() )
      return itr->second;
    else
      throw InvalidPath(FromHere(), "No component exists with path [" + path.string() + "]");
  }

  void CRoot::define_component_path( const CPath& path , boost::shared_ptr<Component> comp )
  {
    cf_assert ( path.is_complete() );

    // remove the current path of the component, if exists
    CompStorage_t::iterator old = m_toc.find( comp->full_path().string() );
    if ( old != m_toc.end() )
      m_toc.erase(old);

    // set the new path
    CompStorage_t::iterator itr = m_toc.find(path.string());
    if ( itr != m_toc.end() )
      throw ValueExists(FromHere(), "A component exists with path [" + path.string() + "]");

    m_toc[path.string()] = comp;
  }

 ////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
