// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QDebug>

#include "Common/XML.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"
#include "GUI/Client/Core/UnknownTypeException.hpp"

#include "GUI/Client/UI/GraphicalBool.hpp"
#include "GUI/Client/UI/GraphicalDouble.hpp"
#include "GUI/Client/UI/GraphicalInt.hpp"
#include "GUI/Client/UI/GraphicalRestrictedList.hpp"
#include "GUI/Client/UI/GraphicalString.hpp"
#include "GUI/Client/UI/GraphicalUri.hpp"
#include "GUI/Client/UI/GraphicalUrlArray.hpp"

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

GraphicalValue * GraphicalValue::createFromOption(CF::Common::Option::ConstPtr option,
                                        QWidget * parent)
{
  GraphicalValue * value = nullptr;
  std::string tag(option->tag());
  std::string type(option->type());

  if(option->has_restricted_list())
    value = new GraphicalRestrictedList(option, parent);
  else if(tag.compare("array") != 0)
  {
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
  else
  {
    OptionArray::ConstPtr array = boost::dynamic_pointer_cast<OptionArray const>(option);

    tag = array->elem_type();

    if(tag.compare(XmlTag<bool>::type()) == 0)               // bool option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<CF::Real>::type()) == 0)      // Real option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<int>::type()) == 0)           // int option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<CF::Uint>::type()) == 0)      // Uint option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<std::string>::type()) == 0)   // string option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<URI>::type()) == 0)           // URI option
      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
    else if(tag.compare(XmlTag<boost::filesystem::path>::type()) == 0)           // path option
      value = new GraphicalUrlArray(parent);
    else
      throw CastingFailed(FromHere(), tag + ": Unknown type");
  }
  return value;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::getValueString() const
{
  QVariant value = this->getValue();

  if(value.type() == QVariant::StringList)
    return value.toStringList().join(":");

  return value.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalValue::getOriginalValue() const
{
  return m_originalValue;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString GraphicalValue::getOriginalValueString() const
{
  if(m_originalValue.type() == QVariant::StringList)
    return m_originalValue.toStringList().join(":");

  return m_originalValue.toString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalValue::isModified() const
{
  return getOriginalValueString() != getValueString();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalValue::commit()
{
  m_originalValue = getValue();
  emit valueChanged();
}
