// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include <QApplication>

#include "rapidxml/rapidxml.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/XML/FileOperations.hpp"

#include "ui/core/NLog.hpp"
#include "ui/core/TreeThread.hpp"

#include "ui/graphics/GraphicalValue.hpp"
#include "ui/graphics/OptionLayout.hpp"

#include "ui/graphics/SignatureDialog.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

SignatureDialog::SignatureDialog( QWidget *parent ) :
    QDialog(parent),
    m_ok_clicked(false),
    m_is_blocking(false)
{
  m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_data_layout = new OptionLayout();
  m_main_layout = new QVBoxLayout(this);

  m_main_layout->addLayout(m_data_layout);
  m_main_layout->addWidget(m_buttons);

  connect(m_buttons, SIGNAL(accepted()), this, SLOT(bt_ok_clicked()));
  connect(m_buttons, SIGNAL(rejected()), this, SLOT(bt_cancel_clicked()));
}

//////////////////////////////////////////////////////////////////////////

SignatureDialog::~SignatureDialog()
{

}

//////////////////////////////////////////////////////////////////////////

bool SignatureDialog::show( XmlNode & sig, const QString & title, bool block )
{
  cf3_assert( sig.is_valid() );

  XmlNode node( sig.content->first_node() );
  QString name;
  std::string str;
  rapidxml::xml_node<> * n = sig.content->parent()->parent()->parent()->parent();

  if(is_not_null(n))
    XML::to_string( n, str );

  m_ok_clicked = false;

  this->setWindowTitle(title);

  m_data_layout->clear_options();

  for( ; node.is_valid() ; node.content = node.content->next_sibling())
  {
    m_data_layout->add( SignalOptions::xml_to_option(node) );

    name = node.content->first_attribute( Protocol::Tags::attr_key() )->value();

    m_nodes[name] = node;
  }

  if( m_data_layout->has_options() )
  {
    if(block)
    {
      m_is_blocking = true;
      this->exec();
    }
    else
    {
      m_is_blocking = false;
      this->setModal(true);
      this->setVisible(true);
    }
  }
  else
  {
    m_ok_clicked = true;
    emit finished(QDialog::Accepted);
  }

  return m_ok_clicked;
}

//////////////////////////////////////////////////////////////////////////

void SignatureDialog::bt_ok_clicked()
{
  m_ok_clicked = true;

  QMap<QString, QString> options;

  m_data_layout->options(options, true);

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

  if(m_is_blocking)
    this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

void SignatureDialog::bt_cancel_clicked()
{
  m_ok_clicked = false;
  emit finished(QDialog::Rejected);

  this->setVisible(false);
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
