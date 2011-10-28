// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "common/Log.hpp"
#include "common/Signal.hpp"

#include "common/BasicExceptions.hpp"
#include "common/CF.hpp"

#include "common/Root.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  Root::Ptr Root::create ( const std::string& name )
  {
    boost::shared_ptr<Root> root(new AllocatedComponent<Root>(name), Deleter< AllocatedComponent<Root> >());

    // put himself in the database
    root->m_toc[root->uri().path()] = root;

    return root;
  }

////////////////////////////////////////////////////////////////////////////////

  Root::Root ( const std::string& name ) : Component ( name )
  {
    // we need to manually register the type name since Root cannot be
    // put into ComponentBuilder because the constructor is private
    TypeInfo::instance().regist<Root>( type_name() );

    regist_signal("new_event")
        ->description( "Notifies new events." );
  }

  Root::~Root()
  {
    std::cout << "goodbye " << FromHere().str() << std::endl;
  }

////////////////////////////////////////////////////////////////////////////////

  Component::Ptr Root::retrieve_component( const URI& path )
  {
    cf3_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
    {
      cf3_assert( is_not_null(itr->second) );
      return itr->second;
    }
    else
      return Component::Ptr();
  }

////////////////////////////////////////////////////////////////////////////////

  Component::Ptr Root::retrieve_component_checked( const URI& path )
  {
    cf3_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
    {
      cf3_assert( is_not_null(itr->second) );
      return itr->second;
    }
    else
      throw InvalidURI(FromHere(), "No component exists with path [" + path.path() + "]");
  }

////////////////////////////////////////////////////////////////////////////////

  void Root::change_component_path( const URI& path , boost::shared_ptr<Component> comp )
  {
    remove_component_path( comp->uri().path() );

    // set the new path
    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
      throw ValueExists(FromHere(), "A component exists with path [" + path.path() + "]");

    m_toc[path.path()] = comp;
  }

////////////////////////////////////////////////////////////////////////////////

  void Root::remove_component_path( const URI& path )
  {
    cf3_assert ( path.is_complete() );
    cf3_assert ( path.scheme() == URI::Scheme::CPATH );

    // remove the current path of the component, if exists
    CompStorage_t::iterator old = m_toc.find( path.path() );
    if ( old != m_toc.end() )
      m_toc.erase(old);
  }

////////////////////////////////////////////////////////////////////////////////

  bool Root::exists_component_path( const URI& path ) const
  {
    cf3_assert ( path.is_complete() );
    cf3_assert ( path.scheme() == URI::Scheme::CPATH );

    return ( m_toc.find( path.path() ) != m_toc.end() );
  }

////////////////////////////////////////////////////////////////////////////////

  std::string Root::list_toc() const
  {
    std::ostringstream out;

    CompStorage_t::const_iterator itr = m_toc.begin();
    for ( ; itr != m_toc.end(); ++itr )
    {
      out << itr->first << " " << itr->second->uri().path() << "\n";
    }

    return out.str();
  }

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
