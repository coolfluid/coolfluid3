// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include "common/XML/SignalFrame.hpp"
#include "common/XML/FileOperations.hpp"

#include "UI/Graphics/SignalInspectorDialog.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

////////////////////////////////////////////////////////////////////////////

SignalInspectorDialog::SignalInspectorDialog(QWidget *parent) :
    QDialog(parent)
{
  m_textArea = new QTextEdit();
  m_buttons = new QDialogButtonBox();

  m_mainLayout = new QVBoxLayout(this);

  m_buttons->addButton(QDialogButtonBox::Ok);

  m_textArea->setWordWrapMode(QTextOption::NoWrap);
  m_textArea->setReadOnly(true);

  m_mainLayout->addWidget(m_textArea);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(close()));
}

////////////////////////////////////////////////////////////////////////////

SignalInspectorDialog::~SignalInspectorDialog()
{
  delete m_textArea;
  delete m_buttons;
  delete m_mainLayout;
}

////////////////////////////////////////////////////////////////////////////

void SignalInspectorDialog::show(const common::XML::SignalFrame & signal)
{
  std::string str;

  XML::to_string(signal.node, str);

  m_textArea->setText(QString(str.c_str()).replace('\t', "  "));

  m_textArea->updateGeometry();
  updateGeometry();
  adjustSize();
  resize(childrenRect().size());

  this->exec();
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
