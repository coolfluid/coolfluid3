// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include "rapidxml/rapidxml.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/Core/NLog.hpp"
#include "UI/Core/TreeThread.hpp"

#include "UI/Graphics/GraphicalValue.hpp"
#include "UI/Graphics/OptionLayout.hpp"

#include "UI/Graphics/SignatureDialog.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

SignatureDialog::SignatureDialog(QWidget *parent) :
    QDialog(parent),
    m_okClicked(false),
    m_isBlocking(false)
{
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_dataLayout = new OptionLayout();
  m_mainLayout = new QVBoxLayout(this);

  m_mainLayout->addLayout(m_dataLayout);
  m_mainLayout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(btOkClicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(btCancelClicked()));
}

//////////////////////////////////////////////////////////////////////////

SignatureDialog::~SignatureDialog()
{

}

//////////////////////////////////////////////////////////////////////////

bool SignatureDialog::show(XmlNode & sig, const QString & title, bool block)
{
  cf_assert( sig.is_valid() );

  XmlNode node = sig.content->first_node();
  QString name;

  m_okClicked = false;

  this->setWindowTitle(title);

  m_dataLayout->clearOptions();

  for( ; node.is_valid() ; node.content = node.content->next_sibling())
  {
    m_dataLayout->addOption( SignalOptions::xml_to_option(node) );

    name = node.content->first_attribute( Protocol::Tags::attr_key() )->value();

    m_nodes[name] = node;
  }

 if( m_dataLayout->hasOptions() )
  {
   if(block)
   {
     m_isBlocking = true;
     this->exec();
   }
   else
   {
     m_isBlocking = false;
     this->setModal(true);
     this->setVisible(true);
   }
  }
  else
  {
    m_okClicked = true;
    emit finished(QDialog::Accepted);
  }

  return m_okClicked;
}

//////////////////////////////////////////////////////////////////////////

void SignatureDialog::btOkClicked()
{
  m_okClicked = true;

  QMap<QString, QString> options;

  m_dataLayout->options(options, true);

  QMap<QString, XmlNode>::iterator it = m_nodes.begin();

  for( ; it != m_nodes.end() ; it++)
  {
    XmlNode & optionNode = it.value();
    if( Map::is_single_value(optionNode) )
    {
      XmlNode node( optionNode.content->first_node() );
      const std::string value = options[it.key()].toStdString().c_str();

      node.content->value( node.content->document()->allocate_string(value.c_str(), value.size()+1), value.size() );
    }
    else
    {
      QStringList value = options[it.key()].split(";");
      QStringList::iterator itValue = value.begin();

      std::string delim(optionNode.content->first_attribute( Protocol::Tags::attr_array_delimiter() )->value());
      std::string val_str;
      rapidxml::xml_node<char>* node = it.value().content;

      for( ; itValue != value.end() ; itValue++)
      {
        if(!val_str.empty())
          val_str += delim;

        val_str += itValue->toStdString();
      }

      node->value( node->document()->allocate_string(val_str.c_str()) );
      it.value().set_attribute(Protocol::Tags::attr_array_size(), QString::number(value.count()).toStdString());
    }
  }

  emit finished(QDialog::Accepted);

  if(m_isBlocking)
    this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SignatureDialog::btCancelClicked()
{
  m_okClicked = false;
  emit finished(QDialog::Rejected);

  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
