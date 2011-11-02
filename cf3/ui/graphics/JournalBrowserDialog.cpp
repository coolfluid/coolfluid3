// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "ui/core/NBrowser.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NJournal.hpp"

#include "ui/graphics/SignalInspectorDialog.hpp"

#include "ui/graphics/JournalBrowserDialog.hpp"

////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

JournalBrowserBuilder & JournalBrowserBuilder::instance()
{
  static JournalBrowserBuilder jbb;
  return jbb;
}

////////////////////////////////////////////////////////////////////////////

void JournalBrowserBuilder::journal_request(bool local)
{
  m_dialog->show(nullptr);
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserBuilder::JournalBrowserBuilder()
{
  m_dialog = new JournalBrowserDialog();

  connect(&JournalNotifier::instance(), SIGNAL(journal_request(bool)),
          this, SLOT(journal_request(bool)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::JournalBrowserDialog(QWidget *parent) :
    QDialog(parent),
    m_model(new NJournalBrowser(nullptr, this))
{
  m_view = new QTableView(this);
  m_buttons = new QDialogButtonBox(this);


  m_main_layout = new QVBoxLayout(this);

  NBrowser::global()->add_node(m_model);

  m_view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->horizontalHeader()->setStretchLastSection(true);
  m_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_view->setModel(m_model.get());
  m_view->setAlternatingRowColors(true);

  m_buttons->addButton(QDialogButtonBox::Ok);
  m_bt_execute = m_buttons->addButton("Execute", QDialogButtonBox::ActionRole);

  m_main_layout->addWidget(m_view);
  m_main_layout->addWidget(m_buttons);

  m_view->updateGeometry();
  updateGeometry();
  adjustSize();
  resize( childrenRect().size() );

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
  connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bt_clicked(QAbstractButton*)));
  connect(m_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(double_clicked(QModelIndex)));
}

////////////////////////////////////////////////////////////////////////////

JournalBrowserDialog::~JournalBrowserDialog()
{
  NBrowser::global()->remove_node(m_model->name().c_str());
  delete m_view;
  delete m_buttons;
  delete m_main_layout;
}

////////////////////////////////////////////////////////////////////////////

void JournalBrowserDialog::show(const XmlNode * rootNode)
{
  m_model->set_root_node(rootNode);
  m_model->request_journal();

//  this->exec();
  this->setModal(true);
  this->setVisible(true);
}

////////////////////////////////////////////////////////////////////////////

void JournalBrowserDialog::double_clicked(const QModelIndex & index)
{
  if(index.isValid())
  {
    SignalInspectorDialog sid;

    sid.show(m_model->signal(index));
  }
}

////////////////////////////////////////////////////////////////////////////

void JournalBrowserDialog::bt_clicked(QAbstractButton *button)
{
  if(button == ((QAbstractButton*) m_bt_execute) )
  {
    try
    {
      m_model->send_exec_signal( m_view->currentIndex() );
    }
    catch(Exception & e)
    {
      NLog::global()->add_message(e.what());
    }
  }
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
