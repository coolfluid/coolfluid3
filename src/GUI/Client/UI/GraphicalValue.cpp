// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QRegExpValidator>
#include <QDebug>

#include "Common/XML.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/UI/GraphicalArray.hpp"
#include "GUI/Client/UI/GraphicalArrayRestrictedList.hpp"
#include "GUI/Client/UI/GraphicalBool.hpp"
#include "GUI/Client/UI/GraphicalDouble.hpp"
#include "GUI/Client/UI/GraphicalInt.hpp"
#include "GUI/Client/UI/GraphicalRestrictedList.hpp"
#include "GUI/Client/UI/GraphicalString.hpp"
#include "GUI/Client/UI/GraphicalUri.hpp"
#include "GUI/Client/UI/GraphicalUriArray.hpp"

#include "GUI/Client/UI/GraphicalValue.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

GraphicalValue::GraphicalValue(QWidget *parent) :
    QWidget(parent),
    m_parent(parent),
    m_committing(false)
{
  this->m_layout = new QHBoxLayout(this);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalValue::~GraphicalValue()
{
  delete m_layout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalValue * GraphicalValue::createFromOption(CF::Common::Option::ConstPtr option,
                                        QWidget * parent)
{
  GraphicalValue * value = nullptr;

  if(option.get() == nullptr)
    return value;

  std::string tag(option->tag());

  if(tag.compare("array") != 0)
  {
    if(option->has_restricted_list())
      value = new GraphicalRestrictedList(option, parent);
    else
    {
      std::string type(option->type());

      if(type.compare(XmlTag<bool>::type()) == 0)               // bool option
        value = new GraphicalBool(option, parent);
      else if(tag.compare(XmlTag<CF::Real>::type()) == 0)      // Real option
        value = new GraphicalDouble(option, parent);
      else if(tag.compare(XmlTag<int>::type()) == 0)           // int option
        value = new GraphicalInt(false, option, parent);
      else if(tag.compare(XmlTag<CF::Uint>::type()) == 0)      // Uint option
        value = new GraphicalInt(true, option, parent);
      else if(tag.compare(XmlTag<std::string>::type()) == 0)   // string option
        value = new GraphicalString(option, parent);
      else if(tag.compare(XmlTag<URI>::type()) == 0)           // URI option
        value = new GraphicalUri(boost::dynamic_pointer_cast<OptionURI const>(option), parent);
      else
        throw CastingFailed(FromHere(), tag + ": Unknown type");
    }
  }
  else
  {
    if(option->has_restricted_list())
      value = new GraphicalArrayRestrictedList(option, parent);
    else
    {
      OptionArray::ConstPtr array = boost::dynamic_pointer_cast<OptionArray const>(option);
      std::string value_str = array->value_str();

      tag = array->elem_type();

      if(tag.compare(XmlTag<bool>::type()) == 0)               // bool option
        value = new GraphicalArray(new QRegExpValidator(QRegExp("(true)|(false)|(1)|(0)"), parent), parent);
      else if(tag.compare(XmlTag<CF::Real>::type()) == 0)      // Real option
      {
        QDoubleValidator * val = new QDoubleValidator(nullptr);
        val->setNotation(QDoubleValidator::ScientificNotation);
        value = new GraphicalArray(val, parent);
      }
      else if(tag.compare(XmlTag<int>::type()) == 0)           // int option
        value = new GraphicalArray(new QIntValidator(), parent);
      else if(tag.compare(XmlTag<CF::Uint>::type()) == 0)      // Uint option
        value = new GraphicalArray(new QIntValidator(0, INT_MAX, parent), parent);
      else if(tag.compare(XmlTag<std::string>::type()) == 0)   // string option
        value = new GraphicalArray(nullptr, parent);
      else if(tag.compare(XmlTag<URI>::type()) == 0)           // URI option
        value = new GraphicalUriArray(parent);
      else
        throw CastingFailed(FromHere(), tag + ": Unknown type");

      ClientRoot::instance().log()->addMessage(QString(value_str.c_str()));

      value->setValue( QString(value_str.c_str()).split("@@") );
    }
  }
  return value;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::valueString() const
{
  QVariant value = this->value();

  if(value.type() == QVariant::StringList)
    return value.toStringList().join("@@");

  return value.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalValue::originalValue() const
{
  return m_originalValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::originalValueString() const
{
  if(m_originalValue.type() == QVariant::StringList)
    return m_originalValue.toStringList().join("@@");

  return m_originalValue.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalValue::isModified() const
{
  return originalValueString() != valueString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalValue::commit()
{
  m_originalValue = value();
  emit valueChanged();
}
