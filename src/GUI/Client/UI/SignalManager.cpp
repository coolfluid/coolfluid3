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

#include "Common/XmlHelpers.hpp"
#include "Common/XmlSignature.hpp"

#include "GUI/Client/UI/SignatureDialog.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

SignalManager::SignalManager(QMainWindow *parent) :
    QObject(parent)
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

void SignalManager::showMenu(const QPoint & pos, const CF::Common::CPath & path,
                             const QList<ActionInfo> & sigs)
{
  QList<ActionInfo>::const_iterator it = sigs.begin();
  m_menu->clear();

  m_path = path;

  for( ; it!= sigs.end() ; it++)
  {
    QAction * action = m_menu->addAction(it->m_name);

    if(!it->m_readableName.isEmpty())
      action->setText(it->m_readableName);

    action->setStatusTip(it->m_description);

    connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
    connect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));

    m_signals[action] = *it;
    m_localStatus[action] = it->m_is_local;
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
    ActionInfo & info = m_signals[action];
    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
    XmlNode & node = *XmlOps::goto_doc_node(*doc.get());
    XmlNode & frame = *XmlOps::add_signal_frame(node, info.m_name.toStdString(),
                                               m_path, m_path, true);
    XmlParams p(frame);

    XmlNode & valuemap = *p.add_valuemap(XmlParams::tag_key_options());

    info.m_signature.put_signature(valuemap);

    try
    {
      SignatureDialog * sg = new SignatureDialog();

      if(sg->show(valuemap, action->text()))
      {
        if(m_localStatus[action])
        {
          try
          {
            if(ClientRoot::root()->root()->full_path().string() == m_path.string())
              ClientRoot::root()->call_signal(info.m_name.toStdString(), frame);
            else
              ClientRoot::root()->root()->access_component(m_path)->call_signal(info.m_name.toStdString(), frame);
          }
          catch(InvalidPath ip)
          {
            ClientRoot::log()->addException(ip.what());
          }
        }
        else
          ClientRoot::core()->sendSignal(*doc);
      }

      delete sg;
    }
    catch( ValueNotFound & vnf)
    {
      ClientRoot::log()->addException(vnf.what());
    }
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
