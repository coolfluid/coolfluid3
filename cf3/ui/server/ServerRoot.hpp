// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Server_ServerRoot_hpp
#define cf3_ui_Server_ServerRoot_hpp

//////////////////////////////////////////////////////////////////////////////

#include "ui/server/CCore.hpp"

template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { namespace PE { class Manager; } }

namespace ui {
namespace server {

//////////////////////////////////////////////////////////////////////////////

  class ServerRoot
  {

  public:

    static ServerRoot & instance();

    Handle< common::Component > root() { return m_root; }

    Handle< common::Component > root() const { return m_root; }

    Handle< CCore > core() { return Handle<CCore>(m_core); }

    Handle< CCore > core() const { return Handle<CCore>(m_core); }

    boost::shared_ptr<common::PE::Manager> manager() { return m_pe_manager; }

    boost::shared_ptr<common::PE::Manager const> manager() const { return m_pe_manager; }

    void process_signal( const std::string & target,
                         const common::URI & receiver,
                         const std::string & clientid,
                         const std::string & frameid,
                         common::SignalArgs & node );

    /// @name SIGNALS
    // @{
    void signal_to_forward( common::SignalArgs & args );

  private:

    ServerRoot();

    ~ServerRoot();

  private: // data

    Handle< common::Component > m_root;

    boost::shared_ptr< CCore > m_core;

    std::vector< common::URI > m_local_components;

    boost::shared_ptr<common::PE::Manager> m_pe_manager;

  }; // class ServerRoot


} // Server
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Server_ServerRoot_hpp
