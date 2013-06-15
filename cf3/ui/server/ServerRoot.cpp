// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Group.hpp"
#include "common/Core.hpp"
#include "common/PE/Manager.hpp"
#include "common/XML/Protocol.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/server/ServerRoot.hpp"

using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::common::XML;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {

//////////////////////////////////////////////////////////////////////////////

ServerRoot::ServerRoot()
  : m_root( Core::instance().root().handle<Component>() ),
    m_core( new CCore() ),
    m_pe_manager( common::allocate_component<Manager>("PEManager") )
{
  m_root->add_component(m_core);

  Handle< Component > tools = m_root->get_child("Tools");

  tools->add_component( m_pe_manager );

  m_local_components.push_back( URI( SERVER_CORE_PATH, URI::Scheme::CPATH ) );

  m_pe_manager->mark_basic();

  m_pe_manager->signal("signal_to_forward")
      ->connect( boost::bind(&ServerRoot::signal_to_forward, this, _1) );

}

//////////////////////////////////////////////////////////////////////////////

ServerRoot::~ServerRoot()
{

}

//////////////////////////////////////////////////////////////////////////////

ServerRoot & ServerRoot::instance()
{
  static ServerRoot root;
  return root;
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::process_signal( const std::string & target,
                                 const URI & receiver,
                                 const std::string & clientid,
                                 const std::string & frameid,
                                 SignalArgs & signal )
{
  std::vector<URI>::iterator it;
  it = std::find(m_local_components.begin(), m_local_components.end(), receiver);

  if( it != m_local_components.end() ) // if the receiver is a local component
  {
    std::string message; // mesage for (N)ACK frame
    bool success = false;

    try
    {
      Handle< Component > comp = m_root->access_component_checked(receiver);

      comp->call_signal(target, signal );

      SignalFrame reply = signal.get_reply();

      if( reply.node.is_valid() )
      {
        m_core->send_signal( signal );
      }

      success = true;
    }
    catch(cf3::common::Exception & cfe)
    {
      message = cfe.what();
    }
    catch(std::exception & stde)
    {
      message = stde.what();
    }
    catch(...)
    {
      message = "Unknown exception thrown during execution of action [" +
          target + "] on component [" + receiver.path() + "].";
    }

    m_core->send_ack( clientid, frameid, success, message );

  }
  else // the receiver is not a local component
  {
    m_pe_manager->send_to( "Workers", signal );
  }
}

//////////////////////////////////////////////////////////////////////////////

void ServerRoot::signal_to_forward( common::SignalArgs & args )
{
  m_core->send_signal( args );
}

//////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

