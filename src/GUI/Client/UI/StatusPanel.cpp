// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QModelIndex>

#include "Common/CF.hpp"

#include "GUI/Client/Core/StatusModel.hpp"

#include "GUI/Client/UI/StatusPanel.hpp"

using namespace CF::GUI::Client;

StatusPanel::StatusPanel(StatusModel * model, QWidget * parent)
: QTreeView(parent)
{
  m_model = model;
  this->setModel(m_model);
//  this->header()->setResizeMode(QHeaderView::ResizeToContents);

  connect(m_model, SIGNAL(subSystemAdded(const QModelIndex &)),
          this, SLOT(subSystemAdded(const QModelIndex &)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

StatusPanel::~StatusPanel()
{
  delete m_model;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void StatusPanel::subSystemAdded(const QModelIndex & index)
{
  this->expand(index.parent());
  this->expand(index);
}
