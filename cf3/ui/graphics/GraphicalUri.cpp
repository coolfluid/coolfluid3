// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStringListModel>

#include "ui/core/NTree.hpp"

#include "ui/graphics/SelectPathDialog.hpp"
#include "ui/graphics/BrowserDialog.hpp"

#include "ui/graphics/GraphicalUri.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

GraphicalUri::GraphicalUri(const boost::shared_ptr<OptionURI>& opt, QWidget *parent) :
    GraphicalValue(parent)
{
  m_bt_browse = new QPushButton("Browse", this);
  m_edit_path = new QLineEdit(this);
  m_completer_model = new QStringListModel(this);
  m_completer = new QCompleter(this);
  m_combo_schemes = new QComboBox(this);

  m_completer->setModel(m_completer_model);

  m_combo_schemes->addItems(QStringList() << "cpath" << "file" << "http");

  scheme_changed(m_combo_schemes->currentText());

  m_layout->addWidget(m_combo_schemes);
  m_layout->addWidget(m_edit_path);
  m_layout->addWidget(m_bt_browse);

  if(opt.get() != nullptr)
  {
    this->set_value(opt->value_str().c_str());
    this->set_schemes(opt->supported_protocols());
  }

  connect(m_bt_browse, SIGNAL(clicked()), this, SLOT(bt_browse_clicked()));
  connect(m_edit_path, SIGNAL(textChanged(QString)), this, SLOT(update_model(QString)));
  connect(m_combo_schemes, SIGNAL(activated(QString)), this, SLOT(scheme_changed(QString)));

  this->update_model("");
}

////////////////////////////////////////////////////////////////////////////

GraphicalUri::~GraphicalUri()
{
  delete m_bt_browse;
  delete m_edit_path;
  delete m_completer;
  delete m_combo_schemes;
}

////////////////////////////////////////////////////////////////////////////

QVariant GraphicalUri::value() const
{
  QString protocol = m_combo_schemes->currentText();
  QString path = m_edit_path->text();
  QString currentScheme = path.left( path.indexOf(':') );

  bool hasProtocol = m_combo_schemes->findText(currentScheme) != -1;

  if(!hasProtocol && !path.isEmpty())
    return QString("%1:%2").arg(protocol).arg(path);

  return path;
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUri::set_schemes(const std::vector<URI::Scheme::Type> & list)
{
  m_combo_schemes->clear();

  if(list.empty())
  {
    m_combo_schemes->addItem("cpath");
    m_combo_schemes->addItem("file");
    m_combo_schemes->addItem("http");
  }
  else
  {
    std::vector<URI::Scheme::Type>::const_iterator it;
    for(it = list.begin() ; it != list.end() ; it++)
    {
      QString scheme( URI::Scheme::Convert::instance().to_str(*it).c_str() );

      if( m_combo_schemes->findText(scheme) == -1 )
        m_combo_schemes->addItem(scheme);
    }

  }

  scheme_changed(m_combo_schemes->currentText());
}

////////////////////////////////////////////////////////////////////////////

bool GraphicalUri::set_value(const QVariant & value)
{
  if(value.type() == QVariant::String)
  {
    URI uriString(value.toString().toStdString());

    m_original_value = uriString.string().c_str();
    m_edit_path->setText(uriString.string().c_str());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUri::bt_browse_clicked()
{
  if(m_combo_schemes->currentText() == "cpath")
  {
    SelectPathDialog spd;
    QString modified_path = m_edit_path->text();
    URI completePath = NTree::global()->complete_relativepath(modified_path.toStdString());

    URI path = spd.show(completePath);

    if(!path.empty())
      m_edit_path->setText(path.string().c_str());
  }
  else if(m_combo_schemes->currentText() == "file")
  {
    BrowserDialog browser;
    QVariant filename;

    if( browser.show(false, filename) && !filename.toString().isEmpty())
      m_edit_path->setText(filename.toString());
  }
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUri::update_model(const QString & path)
{
  emit value_changed();
}

////////////////////////////////////////////////////////////////////////////

void GraphicalUri::scheme_changed(const QString & type)
{
  m_bt_browse->setVisible(type == "cpath" || type == "file");
  m_edit_path->setVisible(!type.isEmpty());
  m_edit_path->setCompleter(type == "cpath" ? m_completer : nullptr);
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
