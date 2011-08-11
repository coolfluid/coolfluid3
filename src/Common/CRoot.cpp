// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "Common/Log.hpp"
#include "Common/Signal.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/CF.hpp"
#include "Common/NotificationQueue.hpp"

#include "Common/CRoot.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  CRoot::Ptr CRoot::create ( const std::string& name )
  {
    CRoot* raw_root = new CRoot(name);
    boost::shared_ptr<CRoot> root ( raw_root );

    // point root's parent and root to himself
    root->m_root = root;
    root->m_raw_parent = raw_root;

    // put himself in the database
    root->m_toc[root->uri().path()] = root;

    return root;
  }

////////////////////////////////////////////////////////////////////////////////

  CRoot::CRoot ( const std::string& name ) : Component ( name )
  {
    // we need to manually register the type name since CRoot cannot be
    // put into ComponentBuilder because the constructor is private
    TypeInfo::instance().regist<CRoot>( type_name() );

    regist_signal("new_event")
        ->description( "Notifies new events." );

    m_path = "/";
  }

  CRoot::~CRoot()
  {
    std::cout << "goodbye " << FromHere().str() << std::endl;
  }

////////////////////////////////////////////////////////////////////////////////

  Component::Ptr CRoot::retrieve_component( const URI& path )
  {
    cf_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
    {
      cf_assert( is_not_null(itr->second) );
      return itr->second;
    }
    else
      return Component::Ptr();
  }

////////////////////////////////////////////////////////////////////////////////

  Component::Ptr CRoot::retrieve_component_checked( const URI& path )
  {
    cf_assert ( path.is_complete() );

    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
    {
      cf_assert( is_not_null(itr->second) );
      return itr->second;
    }
    else
      throw InvalidURI(FromHere(), "No component exists with path [" + path.path() + "]");
  }

////////////////////////////////////////////////////////////////////////////////

  void CRoot::change_component_path( const URI& path , boost::shared_ptr<Component> comp )
  {
    remove_component_path( comp->uri().path() );

    // set the new path
    CompStorage_t::iterator itr = m_toc.find(path.path());
    if ( itr != m_toc.end() )
      throw ValueExists(FromHere(), "A component exists with path [" + path.path() + "]");

    m_toc[path.path()] = comp;
  }

////////////////////////////////////////////////////////////////////////////////

  void CRoot::remove_component_path( const URI& path )
  {
    cf_assert ( path.is_complete() );
    cf_assert ( path.scheme() == URI::Scheme::CPATH );

    // remove the current path of the component, if exists
    CompStorage_t::iterator old = m_toc.find( path.path() );
    if ( old != m_toc.end() )
      m_toc.erase(old);
  }

////////////////////////////////////////////////////////////////////////////////

  bool CRoot::exists_component_path( const URI& path ) const
  {
    cf_assert ( path.is_complete() );
    cf_assert ( path.scheme() == URI::Scheme::CPATH );

    return ( m_toc.find( path.path() ) != m_toc.end() );
  }

////////////////////////////////////////////////////////////////////////////////

  std::string CRoot::list_toc() const
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

  void CRoot::raise_new_event ( const std::string & event_name,
                                const URI & raiser_path )
  {
    cf_assert( exists_component_path(raiser_path) );

    std::vector<NotificationQueue*>::iterator it;

    for( it = m_notif_queues.begin() ; it != m_notif_queues.end() ; it++)
      (*it)->add_notification(event_name, raiser_path);
  }


////////////////////////////////////////////////////////////////////////////////

  void CRoot::add_notification_queue ( NotificationQueue * queue )
  {
    cf_assert( queue != nullptr );

    m_notif_queues.push_back(queue);
  }

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
