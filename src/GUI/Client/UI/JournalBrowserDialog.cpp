// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QTableView>
#include <QVBoxLayout>

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/NJournal.hpp"
#include "GUI/Client/Core/NJournalBrowser.hpp"

#include "GUI/Client/UI/SignalInspectorDialog.hpp"

#include "GUI/Client/UI/JournalBrowserDialog.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

namespace CF {
namespace GUI {
namespace ClientUI {

JournalBrowserBuilder & tmp = JournalBrowserBuilder::instance();


JournalBrowserBuilder & JournalBrowserBuilder::instance()
{
  static JournalBrowserBuilder jbb;
  return jbb;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserBuilder::newJournal(/*NJournal * journal,*/ XmlNode & node)
{
  JournalBrowserDialog jbd;

  jbd.show(&node);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserBuilder::JournalBrowserBuilder()
{
  connect(&JournalNotifier::instance(), SIGNAL(newJournal(/*CF::GUI::ClientCore::NJournal*,*/Common::XmlNode&)),
          this, SLOT(newJournal(/*CF::GUI::ClientCore::NJournal*,*/Common::XmlNode&)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::JournalBrowserDialog(QWidget *parent) :
    QDialog(parent)
{
  m_view = new QTableView();
  m_buttons = new QDialogButtonBox();

  m_mainLayout = new QVBoxLayout(this);

  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_buttons->addButton(QDialogButtonBox::Ok);

  m_mainLayout->addWidget(m_view);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));

  m_view->setModel(nullptr); // no model
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserDialog::~JournalBrowserDialog()
{
  delete m_view;
  delete m_buttons;
  delete m_mainLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::show(const Common::XmlNode *rootNode)
{
  NJournalBrowser model(rootNode->first_node(), this);

  m_view->setModel(&model);

  this->exec();

  m_view->setModel(nullptr);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::doubleClicked(const QModelIndex & index)
{
  NJournalBrowser * model = (NJournalBrowser*)m_view->model();

  if(index.isValid() && model != nullptr)
  {
    SignalInspectorDialog sid;

    sid.show(model->signal(index));
  }
}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
