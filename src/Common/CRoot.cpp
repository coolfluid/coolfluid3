// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "Common/BasicExceptions.hpp"
#include "Common/CF.hpp"
#include "Common/NotificationQueue.hpp"

#include "Common/CRoot.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  boost::shared_ptr<CRoot> CRoot::create ( const std::string& name )
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

  CRoot::CRoot ( const std::string& name ) : Component ( name )
  {
    // we need to manually register the type name since CRoot cannot be
    // put into ComponentBuilder because the constructor is private
    TypeInfo::instance().regist<CRoot>( type_name() );

    regist_signal("new_event", "Notifies new events.");

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

  bool CRoot::exists_component_path( const CPath& path ) const
  {
    cf_assert ( path.is_complete() );

    return ( m_toc.find( path.string() ) != m_toc.end() );
  }

////////////////////////////////////////////////////////////////////////////////

  std::string CRoot::list_toc() const
  {
    std::ostringstream out;

    CompStorage_t::const_iterator itr = m_toc.begin();
    for ( ; itr != m_toc.end(); ++itr )
    {
      out << itr->first << " " << itr->second->full_path() << "\n";
    }

    return out.str();
  }

////////////////////////////////////////////////////////////////////////////////

  void CRoot::raise_new_event ( const std::string & event_name,
                                const CPath & raiser_path )
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
