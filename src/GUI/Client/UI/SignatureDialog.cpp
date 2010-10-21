// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/UI/GraphicalValue.hpp"
#include "GUI/Client/UI/OptionLayout.hpp"

#include "GUI/Client/UI/SignatureDialog.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

SignatureDialog::SignatureDialog(QWidget *parent) :
    QDialog(parent),
    m_okClicked(false)
{
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_dataLayout = new OptionLayout();
  m_mainLayout = new QVBoxLayout(this);

  m_mainLayout->addLayout(m_dataLayout);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SignatureDialog::~SignatureDialog()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SignatureDialog::show(XmlNode & sig, const QString & title)
{
  XmlNode * node;
  QString name;

  m_okClicked = false;

  this->setWindowTitle(title);

  m_dataLayout->clearOptions();
  m_nodes.clear();

  for(node = sig.first_node() ; node != nullptr ; node = node->next_sibling())
  {
    m_dataLayout->addOption( CNode::makeOption(*node) );

    name = node->first_attribute(XmlParams::tag_attr_key())->value();

    m_nodes[name] = node;
  }

  if( m_dataLayout->hasOptions() )
  {
    this->exec();

    if(m_okClicked)
    {
      QMap<QString, QString> options;

      m_dataLayout->getOptions(options, true);

      QMap<QString, XmlNode*>::iterator it = m_nodes.begin();

      for( ; it != m_nodes.end() ; it++)
      {
        XmlNode * node = it.value()->first_node();
        const char * value = options[it.key()].toStdString().c_str();

        node->value( node->document()->allocate_string(value) );
      }
    }

  }
  else
    m_okClicked = true;

  return m_okClicked;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignatureDialog::btOkClicked()
{
  m_okClicked = true;
  this->setVisible(false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SignatureDialog::btCancelClicked()
{
  m_okClicked = false;
  this->setVisible(false);
}
