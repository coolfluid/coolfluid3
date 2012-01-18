// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

#include "ui/core/CommitDetails.hpp"

#include "ui/graphics/ConfirmCommitDialog.hpp"

using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

ConfirmCommitDialog::ConfirmCommitDialog(QWidget * parent)
: QDialog(parent)
{
  this->setWindowTitle("Commit confirm");

  m_lab_text = new QLabel("Options have been modified but were not comitted.<br>"
                               "Click on \"<i>Details</i>\" to see what "
                               "modifications have been done.", this);


  m_main_layout = new QVBoxLayout(this);

  m_button_box = new QDialogButtonBox(this);

  m_details_view = new QTableView(this);

  this->create_button("Cancel", CANCEL, QDialogButtonBox::RejectRole);
  this->create_button("Commit", COMMIT, QDialogButtonBox::YesRole);
  this->create_button("Do not commit", DONT_COMMIT, QDialogButtonBox::NoRole);

  m_main_layout->addWidget(m_lab_text);
  m_main_layout->addWidget(m_details_view);
  m_main_layout->addWidget(m_button_box);
}

//////////////////////////////////////////////////////////////////////////

ConfirmCommitDialog::~ConfirmCommitDialog()
{
//  QHash<CommitConfirmation, QPushButton *>::iterator it = m_buttons.begin();

//  for( ; it != m_buttons.end() ; )
//    delete it.value();

//  m_buttons.clear();

//  delete m_mainLayout;
//  delete m_buttonBox;
//  delete m_detailsView;
//  delete m_mainLayout;
}

//////////////////////////////////////////////////////////////////////////

ConfirmCommitDialog::CommitConfirmation ConfirmCommitDialog::show(CommitDetails & commitDetails)
{
  m_answer = CANCEL;

  if(commitDetails.has_options())
  {
    m_details_view->setModel(&commitDetails);
    this->exec();
  }

  return m_answer;
}

//////////////////////////////////////////////////////////////////////////

void ConfirmCommitDialog::button_clicked()
{
  QPushButton * button = static_cast<QPushButton *> (sender());

  if(button != nullptr)
    m_answer = m_buttons.key(button);
  else
    m_answer = CANCEL;

  this->hide();
}

//////////////////////////////////////////////////////////////////////////

void ConfirmCommitDialog::create_button(const QString & text,
                                       CommitConfirmation commConf,
                                       QDialogButtonBox::ButtonRole role)
{
  QPushButton * button = m_button_box->addButton(text, role);
  connect(button, SIGNAL(clicked()), this, SLOT(button_clicked()));
  m_buttons[commConf] = button;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
