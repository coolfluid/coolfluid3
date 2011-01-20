// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Client/UI/SignalManager.hpp"

#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QStatusBar>

#include <QDebug>

#include "Common/URI.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/XmlSignature.hpp"

#include "GUI/Client/UI/SignatureDialog.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

/////////////////////////////////////////////////////////////////////////


SignalManager::SignalManager(QMainWindow *parent) :
    QObject(parent),
    m_currentAction(nullptr),
    m_waitingForSignature(false)
{
  m_menu = new QMenu();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SignalManager::~SignalManager()
{
  m_menu->clear();
  delete m_menu;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignalManager::showMenu(const QPoint & pos, CNode::Ptr node,
                             const QList<ActionInfo> & sigs)
{
  QList<ActionInfo>::const_iterator it = sigs.begin();

  cf_assert( node.get() != nullptr );

  m_menu->clear();

  m_node = node;
  m_currentAction = nullptr;

  connect(node->notifier(), SIGNAL(signalSignature(Common::XmlNode&)),
          this, SLOT(signalSignature(Common::XmlNode&)));

  for( ; it!= sigs.end() ; it++)
  {
    if(!it->m_readableName.isEmpty())
    {
      QAction * action = m_menu->addAction(it->m_readableName);

      action->setStatusTip(it->m_description);

      connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
      connect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));

      m_signals[action] = *it;
      m_localStatus[action] = it->m_isLocal;
    }
  }

  if(!m_menu->isEmpty())
    m_menu->exec(pos);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignalManager::actionTriggered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != nullptr)
  {
    m_currentAction = action;
    m_waitingForSignature = true;

    m_node->requestSignalSignature( m_signals[action].m_name );

//    URI path = m_node->full_path();
//    ActionInfo & info = m_signals[action];
//    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
//    XmlNode & node = *XmlOps::goto_doc_node(*doc.get());
//    XmlNode & frame = *XmlOps::add_signal_frame(node, info.m_name.toStdString(),
//                                                path, path, true);
//    XmlParams p(frame);

//    XmlNode & map = *p.add_map(XmlParams::tag_key_options());

//    info.m_signature.put_signature(map);

//    try
//    {
//      SignatureDialog * sg = new SignatureDialog();

//      if(sg->show(map, action->text()))
//      {
//        if(m_localStatus[action])
//        {
//          try
//          {
//            m_node->call_signal(info.m_name.toStdString(), frame);
//          }
//          catch(InvalidURI ip)
//          {
//            ClientRoot::instance().log()->addException(ip.what());
//          }
//        }
//        else
//          ClientRoot::instance().core()->sendSignal(*doc);
//      }

//      delete sg;
//    }
//    catch( Exception & e)
//    {
//      ClientRoot::instance().log()->addException(e.what());
//    }
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignalManager::actionHovered()
{
  QAction * action = static_cast<QAction*>(sender());

  if(action != nullptr)
  {
    static_cast<QMainWindow*>(parent())->statusBar()->showMessage(action->statusTip(), 3000);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignalManager::signalSignature(XmlNode & node)
{
  if(m_waitingForSignature)
  {
    URI path = m_node->full_path();
    ActionInfo & info = m_signals[m_currentAction];
    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
    XmlNode & doc_node = *XmlOps::goto_doc_node(*doc.get());
    XmlNode & frame = *XmlOps::add_signal_frame(doc_node, info.m_name.toStdString(),
                                                path, path, true);
    XmlParams p(frame);
    XmlParams originalp(node);

    if(originalp.option_map != nullptr)
    {
      XmlNode & map = *p.add_map(XmlParams::tag_key_options());

      //  info.m_signature.put_signature(map);

      XmlOps::deep_copy(*originalp.option_map, map);

      try
      {
        SignatureDialog * sg = new SignatureDialog();

        if(sg->show(map, m_currentAction->text()))
        {
          if(m_localStatus[m_currentAction])
          {
            try
            {
              m_node->call_signal(info.m_name.toStdString(), frame);
            }
            catch(InvalidURI ip)
            {
              ClientRoot::instance().log()->addException(ip.what());
            }
          }
          else
            ClientRoot::instance().core()->sendSignal(*doc);
        }

        delete sg;
      }
      catch( Exception & e)
      {
        ClientRoot::instance().log()->addException(e.what());
      }

    }

    m_waitingForSignature = false;
  }

}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

//////////////////////////////////////////////////////////////////////////////
