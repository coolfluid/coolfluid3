// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/Core/NJournal.hpp"
#include "GUI/Client/Core/NJournalBrowser.hpp"
#include "GUI/Client/Core/SignalNode.hpp"

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

void JournalBrowserBuilder::newJournal(/*NJournal * journal,*/ XmlNode * node)
{
  JournalBrowserDialog * jbd = new JournalBrowserDialog();

  jbd->show(node);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

JournalBrowserBuilder::JournalBrowserBuilder()
{
  connect(&JournalNotifier::instance(), SIGNAL(newJournal(/*CF::GUI::ClientCore::NJournal*,*/Common::XmlNode*)),
          this, SLOT(newJournal(/*CF::GUI::ClientCore::NJournal*,*/Common::XmlNode*)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::JournalBrowserDialog(QWidget *parent) :
    QDialog(parent)
{
  m_view = new QTableView(this);
  m_buttons = new QDialogButtonBox(this);

  m_mainLayout = new QVBoxLayout(this);

  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->horizontalHeader()->setStretchLastSection(true);
  m_view->setModel(nullptr); // no model
  m_view->setAlternatingRowColors(true);

  m_buttons->addButton(QDialogButtonBox::Ok);
  m_btExecute = m_buttons->addButton("Execute", QDialogButtonBox::ActionRole);

  m_mainLayout->addWidget(m_view);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
  connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(btClicked(QAbstractButton*)));
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));
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
  NJournalBrowser * model = new NJournalBrowser(rootNode->first_node(), this);

  m_view->setModel(model);

  m_view->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  m_view->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

  m_view->updateGeometry();
  updateGeometry();
  adjustSize();
  resize(childrenRect().size());

  this->exec();
//  this->setVisible(true);

//  m_view->setModel(nullptr);
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void JournalBrowserDialog::btClicked(QAbstractButton *button)
{
  if(button != (QAbstractButton*)m_btExecute)
  {
    QModelIndex index = m_view->currentIndex();

    try
    {
    if(index.isValid())
    {
      ClientRoot::instance().log()->addMessage("Re-executing the signal.");
      boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
      XmlNode & frameNode = *XmlOps::add_node_to(*XmlOps::goto_doc_node(*doc.get()), "tmp");
      NJournalBrowser * model = (NJournalBrowser*)m_view->model();
      XmlOps::deep_copy(*model->signal(index).node(), frameNode);

      std::string str;
      XmlOps::xml_to_string(*doc.get(), str);

      ClientRoot::instance().log()->addMessage(QString("--->") + str.c_str());

      ClientRoot::instance().core()->sendSignal(*doc.get());
    }
  }catch(Exception & e)
    {
    ClientRoot::instance().log()->addMessage(e.what());
  }
  }
}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
