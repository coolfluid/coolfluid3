// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDebug>

#include "rapidxml/rapidxml.hpp"

#include "GUI/Client/Core/TreeThread.hpp"

#include "GUI/Client/UI/GraphicalValue.hpp"
#include "GUI/Client/UI/OptionLayout.hpp"

#include "GUI/Client/UI/SignatureDialog.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
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
  cf_assert( sig.is_valid() );

  XmlNode node = sig.content->first_node();
  QString name;
  QMap<QString, XmlNode> nodes;

  m_okClicked = false;

  this->setWindowTitle(title);

  m_dataLayout->clearOptions();

  for( ; node.is_valid() ; node.content = node.content->next_sibling())
  {
    m_dataLayout->addOption( CNode::makeOption(node) );

    name = node.content->first_attribute( Protocol::Tags::attr_key() )->value();

    nodes[name] = node;
  }

  if( m_dataLayout->hasOptions() )
  {
    this->exec();

    if(m_okClicked)
    {
      QMap<QString, QString> options;

      m_dataLayout->options(options, true);

      QMap<QString, XmlNode>::iterator it = nodes.begin();

      for( ; it != nodes.end() ; it++)
      {
        XmlNode & optionNode = it.value();
        if( Map::is_single_value(optionNode) )
        {
          XmlNode node( optionNode.content->first_node() );
          const char * value = options[it.key()].toStdString().c_str();

          node.content->value( node.content->document()->allocate_string(value) );
        }
        else
        {
          QStringList value = options[it.key()].split("@@");
          QStringList::iterator itValue = value.begin();

          std::string delim(optionNode.content->first_attribute( Protocol::Tags::attr_array_delimiter() )->value());
          std::string val_str;


//          optionNode.remove_all_nodes();

          for( ; itValue != value.end() ; itValue++)
          {
            if(!val_str.empty())
              val_str += delim;

            val_str += itValue->toStdString();
          }
//            XmlOps::add_node_to(optionNode, "e", itValue->toStdString());

        }
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
