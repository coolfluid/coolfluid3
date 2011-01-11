// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/SignalNode.hpp"

#include "GUI/Client/UI/SignalInspectorDialog.hpp"

using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////

SignalInspectorDialog::SignalInspectorDialog(QWidget *parent) :
    QDialog(parent)
{
  m_textArea = new QTextEdit();
  m_buttons = new QDialogButtonBox();

  m_mainLayout = new QVBoxLayout(this);

  m_buttons->addButton(QDialogButtonBox::Ok);

  m_mainLayout->addWidget(m_textArea);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SignalInspectorDialog::~SignalInspectorDialog()
{
  delete m_textArea;
  delete m_buttons;
  delete m_mainLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignalInspectorDialog::show(const ClientCore::SignalNode & node)
{
  std::string str;

  XmlOps::xml_to_string(*node.node(), str);

  m_textArea->setText(str.c_str());

  this->exec();
}

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
