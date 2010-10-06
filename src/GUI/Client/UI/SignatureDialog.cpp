// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/UI/GraphicalOption.hpp"

#include "GUI/Client/UI/SignatureDialog.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

SignatureDialog::SignatureDialog(QWidget *parent) :
    QDialog(parent),
    m_okClicked(false)
{
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_dataLayout = new QFormLayout();
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
  XmlNode * node = sig.first_node();
  GraphicalOption * goption;
  OptionType::Type type = OptionType::INVALID;

  m_okClicked = false;

  this->setWindowTitle(title);

  while(node != CFNULL)
  {
    XmlAttr * key_attr = node->first_attribute( XmlParams::tag_attr_key() );
    XmlAttr * desc_attr = node->first_attribute( XmlParams::tag_attr_descr() );
    QString name = key_attr != CFNULL ? key_attr->value() : "";
    QString desc = desc_attr != CFNULL ? desc_attr->value() : "";

    if(name.isEmpty())
      throw ValueNotFound(FromHere(), "No non-empty name found.");

    if( std::strcmp(node->name(), XmlParams::tag_node_value()) == 0 )
    {
      XmlNode * type_name = node->first_node();

      if( type_name != CFNULL )
        type = OptionType::Convert::to_enum(type_name->name());
      else
        throw ValueNotFound(FromHere(), "Type not found");
    }
    else if( std::strcmp(node->name(), "array") == 0 )
    {
      XmlAttr * type_attr = node->first_attribute( XmlParams::tag_attr_type() );

      if( type_attr != CFNULL )
      {
        const char * type_str = type_attr->value();

        if(std::strcmp(type_str, "file") == 0)
          type = OptionType::TYPE_FILES;
        else
          throw ValueNotFound(FromHere(), std::string(type_str) + ": Type not found for array");
      }
      else
        throw ValueNotFound(FromHere(), "Type not found");
    }

    goption = new GraphicalOption(type, this);

    goption->setName(name);
    goption->setToolTip(desc);
    goption->addToLayout(m_dataLayout);

    m_data[node] = goption;

    node = node->next_sibling();
  }

  if( !m_data.isEmpty() )
  {
    this->exec();

    if(m_okClicked)
    {
      QMap<XmlNode *, GraphicalOption *>::iterator it = m_data.begin();

      for( ; it != m_data.end() ; it++)
      {
        XmlNode * node = it.key()->first_node();
        const char * value = it.value()->getValueString().toStdString().c_str();

        node->value( node->document()->allocate_string(value) );
        ClientRoot::log()->addMessage(it.value()->getValueString());
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
