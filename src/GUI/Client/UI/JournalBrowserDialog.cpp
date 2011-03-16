// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "GUI/Client/Core/NBrowser.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NJournal.hpp"

#include "GUI/Client/UI/SignalInspectorDialog.hpp"

#include "GUI/Client/UI/JournalBrowserDialog.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;

namespace CF {
namespace GUI {
namespace ClientUI {

JournalBrowserBuilder & JournalBrowserBuilder::instance()
{
  static JournalBrowserBuilder jbb;
  return jbb;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserBuilder::journalRequest(bool local)
{
  m_dialog->show(nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserBuilder::JournalBrowserBuilder()
{
  m_dialog = new JournalBrowserDialog();

  connect(&JournalNotifier::instance(), SIGNAL(journalRequest(bool)),
          this, SLOT(journalRequest(bool)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::JournalBrowserDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new NJournalBrowser(nullptr, this))
{
  m_view = new QTableView(this);
  m_buttons = new QDialogButtonBox(this);


  m_mainLayout = new QVBoxLayout(this);

  NBrowser::globalBrowser()->addNode(m_model);

  m_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->horizontalHeader()->setStretchLastSection(true);
  m_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setModel(m_model.get());
  m_view->setAlternatingRowColors(true);

  m_buttons->addButton(QDialogButtonBox::Ok);
  m_btExecute = m_buttons->addButton("Execute", QDialogButtonBox::ActionRole);

  m_mainLayout->addWidget(m_view);
  m_mainLayout->addWidget(m_buttons);

  m_view->updateGeometry();
  updateGeometry();
  adjustSize();
  resize( childrenRect().size() );

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
  connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(btClicked(QAbstractButton*)));
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserDialog::~JournalBrowserDialog()
{
  NBrowser::globalBrowser()->removeNode(m_model->name().c_str());
  delete m_view;
  delete m_buttons;
  delete m_mainLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::show(const XmlNode * rootNode)
{
  m_model->setRootNode(rootNode);
  m_model->requestJournal();

//  this->exec();
  this->setModal(true);
  this->setVisible(true);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::doubleClicked(const QModelIndex & index)
{
  if(index.isValid())
  {
    SignalInspectorDialog sid;

    sid.show(m_model->signal(index));
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::btClicked(QAbstractButton *button)
{
  if(button == ((QAbstractButton*) m_btExecute) )
  {
    try
    {
      m_model->sendExecSignal( m_view->currentIndex() );
    }
    catch(Exception & e)
    {
      NLog::globalLog()->addMessage(e.what());
    }
  }
}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
