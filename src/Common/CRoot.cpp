// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CF.hpp"

#include "Common/CRoot.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<CRoot> CRoot::create ( const CName& name )
  {
    CRoot* raw_root = new CRoot(name);
    boost::shared_ptr<CRoot> root ( raw_root );

    // point root's parent and root to himself
    root->m_root = root;
    root->m_raw_parent = raw_root;

    // put himself in the database
    root->m_toc[root->full_path().string()] = root;

    return root;
  }

////////////////////////////////////////////////////////////////////////////////

  CRoot::CRoot ( const CName& name ) : Component ( name )
  {
    BUILD_COMPONENT;

    // we need to manually register the type name since CRoot cannot be
    // put into ObjectProvider (because the constructor is private)
    TypeInfo::instance().regist<CRoot>( type_name() );

    m_path = "/";
  }

  CRoot::~CRoot()
  {
  }

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<Component> CRoot::access_component( const CPath& path )
  {
    cf_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_toc.find(path.string());
    if ( itr != m_toc.end() )
      return itr->second;
    else
      throw InvalidPath(FromHere(), "No component exists with path [" + path.string() + "]");
  }

////////////////////////////////////////////////////////////////////////////////

  void CRoot::change_component_path( const CPath& path , boost::shared_ptr<Component> comp )
  {
    remove_component_path( comp->full_path().string() );

    // set the new path
    CompStorage_t::iterator itr = m_toc.find(path.string());
    if ( itr != m_toc.end() )
      throw ValueExists(FromHere(), "A component exists with path [" + path.string() + "]");

    m_toc[path.string()] = comp;
  }

////////////////////////////////////////////////////////////////////////////////

  void CRoot::remove_component_path( const CPath& path )
  {
    cf_assert ( path.is_complete() );

    // remove the current path of the component, if exists
    CompStorage_t::iterator old = m_toc.find( path.string() );
    if ( old != m_toc.end() )
      m_toc.erase(old);
  }

////////////////////////////////////////////////////////////////////////////////

  bool CRoot::exists_component_path( const CPath& path )
  {
    cf_assert ( path.is_complete() );

    return ( m_toc.find( path.string() ) != m_toc.end() );
  }

////////////////////////////////////////////////////////////////////////////////

  std::string CRoot::list_toc() const
  {
    std::stringstream out;

    CompStorage_t::const_iterator itr = m_toc.begin();
    for ( ; itr != m_toc.end(); ++itr )
    {
      out << itr->first << " " << itr->second->full_path() << "\n";
    }

    return out.str();
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
