// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMenu>
#include <QToolBar>

#include "Common/CF.hpp"

#include "GUI/Client/MenuActionInfo.hpp"

using namespace CF::GUI::Client;

MenuActionInfo::MenuActionInfo()
{
  this->initDefaults();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void MenuActionInfo::initDefaults()
{
  m_text = "";
  m_enabled = true;
  m_checkable = false;
  m_icon = QIcon();
  m_shortcut = QKeySequence();

  m_slot = CFNULL;
  m_menu = CFNULL;
  m_toolbar = CFNULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QAction * MenuActionInfo::buildAction(QObject * parent)
{
  QAction * action = CFNULL;

  if(parent != CFNULL)
  {
    action = new QAction(m_text, parent);
    action->setEnabled(m_enabled);

    if(!m_shortcut.isEmpty())
      action->setShortcut(m_shortcut);

    if(m_slot != CFNULL)
      connect(action, SIGNAL(triggered()), parent, m_slot);

    if(m_menu != CFNULL)
      m_menu->addAction(action);

    if(m_toolbar != CFNULL)
      m_toolbar->addAction(action);

    action->setIcon(m_icon);
    action->setCheckable(m_checkable);
    action->setIconVisibleInMenu(true);
  }

  return action;
}
