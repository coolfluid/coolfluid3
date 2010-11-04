// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <QSpinBox>

#include "GUI/Client/UI/ConnectionDialog.hpp"
#include "GUI/Client/Core/TSshInformation.hpp"

using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

ConnectionDialog::ConnectionDialog(QMainWindow * parent)
: QDialog(parent)
{
  QString username;
  QRegExp regex("^USER=");
  QStringList environment = QProcess::systemEnvironment().filter(regex);

  if(environment.size() == 1)
    username = environment.at(0);

  this->setWindowTitle("Connect to server");

  // create the components
  m_labHostname = new QLabel("Hostname:");
  m_labPortNumber = new QLabel("Port number:");

  m_editHostname = new QLineEdit(this);
  m_spinPortNumber = new QSpinBox(this);

  m_infosLayout = new QHBoxLayout();

  m_layout = new QFormLayout(this);
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

  // the dialog is modal
  this->setModal(true);

  m_spinPortNumber->setMinimum(49150); // below 49150, ports are reserved (or not recommended to be used)
  m_spinPortNumber->setMaximum(65535);

  m_editHostname->setText("localhost");
  m_spinPortNumber->setValue(62784);

  // add the components to the m_layout
  m_infosLayout->addWidget(m_labHostname);
  m_infosLayout->addWidget(m_editHostname);
  m_infosLayout->addWidget(m_labPortNumber);
  m_infosLayout->addWidget(m_spinPortNumber);

  m_layout->addRow(m_infosLayout);
  m_layout->addRow(m_buttons);

  // add the layout to the dialog
  this->setLayout(m_layout);

  // connect useful signals to slots
  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ConnectionDialog::~ConnectionDialog()
{
  delete m_buttons;
  delete m_editHostname;
  delete m_infosLayout;
  delete m_labHostname;
  delete m_labPortNumber;
  delete m_layout;
  delete m_spinPortNumber;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ConnectionDialog::show(bool hidePort, TSshInformation & sshInfos)
{
  m_okClicked = false;

  m_labPortNumber->setVisible(!hidePort);
  m_spinPortNumber->setVisible(!hidePort);
  this->adjustSize();

  this->exec();

  if(m_okClicked)
  {
    sshInfos.m_hostname = m_editHostname->text();
    sshInfos.m_port = m_spinPortNumber->value();
  }

  return m_okClicked;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ConnectionDialog::setSshInfos(const TSshInformation & sshInfos)
{
  m_editHostname->setText(sshInfos.m_hostname);
  m_spinPortNumber->setValue(sshInfos.m_port);
}


 // SLOTS

void ConnectionDialog::btOkClicked()
{
  m_okClicked = true;
  this->setVisible(false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ConnectionDialog::btCancelClicked()
{
  this->setVisible(false);
}
