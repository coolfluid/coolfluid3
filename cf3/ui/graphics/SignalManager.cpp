// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QStatusBar>

#include "common/Signal.hpp"
#include "common/URI.hpp"

#include "common/XML/Protocol.hpp"

#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NLog.hpp"

#include "ui/graphics/SignatureDialog.hpp"

#include "ui/graphics/SignalManager.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

/////////////////////////////////////////////////////////////////////////


SignalManager::SignalManager(QMainWindow *parent) :
    QObject(parent),
    m_current_action(nullptr),
    m_waiting_for_signature(false)
{
  m_menu = new QMenu();
}

////////////////////////////////////////////////////////////////////////////

SignalManager::~SignalManager()
{
  m_menu->clear();
  delete m_menu;
}

////////////////////////////////////////////////////////////////////////////

void SignalManager::show_menu(const QPoint & pos, Handle< CNode > node,
                             const QList<ActionInfo> & sigs)
{
  QList<ActionInfo>::const_iterator it = sigs.begin();
  bool is_local = false;

  cf3_assert( node.get() != nullptr );

  m_menu->clear();

  m_node = node;
  m_current_action = nullptr;

  node->signal("signal_signature")
      ->connect( boost::bind(&SignalManager::signal_signature, this, _1) );

//  connect(node->notifier(), SIGNAL(signalSignature(cf3::common::SignalArgs*)),
//          this, SLOT(signalSignature(cf3::common::SignalArgs*)));

  for( ; it!= sigs.end() ; it++)
  {
    if(!it->readable_name.isEmpty())
    {
      if(is_local != it->is_local && it != sigs.begin())
        m_menu->addSeparator();

      QAction * action = m_menu->addAction(it->readable_name);

      action->setStatusTip(it->description);
      action->setEnabled(it->is_enabled);

      connect(action, SIGNAL(triggered()), this, SLOT(action_triggered()));
      connect(action, SIGNAL(hovered()), this, SLOT(action_hovered()));

      m_signals[action] = *it;
      m_local_status[action] = it->is_local;
    }

    is_local = it->is_local;
  }

  if(!m_menu->isEmpty())
    m_menu->exec(pos);
}

////////////////////////////////////////////////////////////////////////////

void SignalManager::action_triggered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != nullptr)
  {
    m_current_action = action;
    m_waiting_for_signature = true;

    if(!m_local_status[action])
      m_node->request_signal_signature( m_signals[action].name );
    else
    {
      SignalFrame frame;
      m_node->local_signature(m_signals[action].name, frame);
      signal_signature(frame);
    }
  }
}

////////////////////////////////////////////////////////////////////////////

void SignalManager::action_hovered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != nullptr)
  {
    static_cast<QMainWindow*>(parent())->statusBar()->showMessage(action->statusTip(), 3000);
  }
}

////////////////////////////////////////////////////////////////////////////

void SignalManager::signal_signature(SignalArgs & args)
{
  if(m_waiting_for_signature)
  {
    URI path = m_node->uri();
    ActionInfo & info = m_signals[m_current_action];
    const char * tag = Protocol::Tags::key_options();

    m_frame = SignalFrame(info.name.toStdString(), path, path);
    SignalFrame& options = m_frame.map( Protocol::Tags::key_options() );

    if( args.has_map(tag) )
      args.map(tag).main_map.content.deep_copy( options.main_map.content );

    try
    {
      SignatureDialog * sg = new SignatureDialog();

      connect(sg, SIGNAL(finished(int)), this, SLOT(dialog_finished(int)));

      sg->show(options.main_map.content, m_current_action->text());
    }
    catch( Exception & e)
    {
      NLog::global()->add_exception(e.what());
    }
    catch( ... )
    {
      NLog::global()->add_exception("Unknown exception caught");
    }



    m_waiting_for_signature = false;
  }

}

////////////////////////////////////////////////////////////////////////////

void SignalManager::dialog_finished(int result)
{
  if(result == QDialog::Accepted)
  {
    SignatureDialog * dlg = static_cast<SignatureDialog*>(sender());

    if(dlg != nullptr)
    {
      if(m_local_status[m_current_action]) // if it is a local signal, call it...
      {
        try
        {
          m_node->call_signal(m_signals[m_current_action].name.toStdString(), m_frame);
        }
        catch(InvalidURI ip)
        {
          NLog::global()->add_exception(ip.what());
        }
      }
      else // ...or send the request to the server
        NetworkQueue::global()->send(m_frame);

    }

    delete dlg;
  }
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
