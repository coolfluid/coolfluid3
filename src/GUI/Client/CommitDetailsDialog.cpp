// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include "GUI/Client/CommitDetails.hpp"

#include "GUI/Client/CommitDetailsDialog.hpp"

using namespace CF::GUI::Client;

CommitDetailsDialog::CommitDetailsDialog(QWidget * parent)
: QDialog(parent)
{
  //CommitDetails details;
  m_mainLayout = new QVBoxLayout(this);
  m_buttonBox = new QDialogButtonBox(this);
  m_view = new QTableView(this);

  m_buttonBox->addButton(QDialogButtonBox::Ok);

  this->resize(this->width(), this->height());

  m_mainLayout->addWidget(m_view, 0);
  m_mainLayout->addWidget(m_buttonBox, 1);

  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(close()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CommitDetailsDialog::~CommitDetailsDialog()
{
 delete m_mainLayout;
 delete m_buttonBox;
 delete m_view;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CommitDetailsDialog::show(CommitDetails & details)
{
  if (details.rowCount() > 0)
  {
    m_view->setModel(&details);

    m_view->resizeColumnsToContents();

    this->exec();
  }
}
