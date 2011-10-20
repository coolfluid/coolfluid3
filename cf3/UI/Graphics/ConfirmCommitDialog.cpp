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

#include "UI/Core/CommitDetails.hpp"

#include "UI/Graphics/ConfirmCommitDialog.hpp"

using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

ConfirmCommitDialog::ConfirmCommitDialog(QWidget * parent)
: QDialog(parent)
{
  this->setWindowTitle("Commit confirm");

  m_labText = new QLabel("Options have been modified but were not comitted.<br>"
                               "Click on \"<i>Details</i>\" to see what "
                               "modifications have been done.", this);


  m_mainLayout = new QVBoxLayout(this);

  m_buttonBox = new QDialogButtonBox(this);

  m_detailsView = new QTableView(this);

  this->createButton("Cancel", CANCEL, QDialogButtonBox::RejectRole);
  this->createButton("Commit", COMMIT, QDialogButtonBox::YesRole);
  this->createButton("Do not commit", DONT_COMMIT, QDialogButtonBox::NoRole);

  m_mainLayout->addWidget(m_labText);
  m_mainLayout->addWidget(m_detailsView);
  m_mainLayout->addWidget(m_buttonBox);
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

  if(commitDetails.hasOptions())
  {
    m_detailsView->setModel(&commitDetails);
    this->exec();
  }

  return m_answer;
}

//////////////////////////////////////////////////////////////////////////

void ConfirmCommitDialog::buttonClicked()
{
  QPushButton * button = static_cast<QPushButton *> (sender());

  if(button != nullptr)
    m_answer = m_buttons.key(button);
  else
    m_answer = CANCEL;

  this->hide();
}

//////////////////////////////////////////////////////////////////////////

void ConfirmCommitDialog::createButton(const QString & text,
                                       CommitConfirmation commConf,
                                       QDialogButtonBox::ButtonRole role)
{
  QPushButton * button = m_buttonBox->addButton(text, role);
  connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
  m_buttons[commConf] = button;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
