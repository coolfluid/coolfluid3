// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QHBoxLayout>
#include <QRegExpValidator>

#include "Math/MathConsts.hpp"

#include "GUI/Client/Core/TreeThread.hpp"
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
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

//////////////////////////////////////////////////////////////////////////

GraphicalValue::GraphicalValue(QWidget *parent) :
    QWidget(parent),
    m_committing(false)
{
  m_layout = new QHBoxLayout(this);

  m_layout->setMargin(0);
}

//////////////////////////////////////////////////////////////////////////

GraphicalValue::~GraphicalValue()
{
  delete m_layout;
}

//////////////////////////////////////////////////////////////////////////

GraphicalValue * GraphicalValue::createFromOption(Option::ConstPtr option,
                                                  QWidget * parent)
{
  GraphicalValue * value = nullptr;

  if(option.get() == nullptr)
    return value;

  std::string tag(option->tag());

  if(tag != "array" )
  {
    if(option->has_restricted_list())
      value = new GraphicalRestrictedList(option, parent);
    else
    {
      std::string type(option->type());

      if(type == Protocol::Tags::type<bool>())               // bool option
        value = new GraphicalBool(option->value<bool>(), parent);
      else if(type == Protocol::Tags::type<Real>())          // Real option
        value = new GraphicalDouble(option->value<Real>(), parent);
      else if(type == Protocol::Tags::type<int>())           // int option
        value = new GraphicalInt(false, option->value<int>(), parent);
      else if(type == Protocol::Tags::type<Uint>())          // Uint option
        value = new GraphicalInt(true, option->value<Uint>(), parent);
      else if(type == Protocol::Tags::type<std::string>())   // string option
        value = new GraphicalString(option->value<std::string>().c_str(), parent);
      else if(type == Protocol::Tags::type<URI>())           // URI option
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
      std::string type(array->elem_type());

      if(type == Protocol::Tags::type<bool>())                 // bool option
      {
        QRegExp regex("(true)|(false)|(1)|(0)|(on)|(off)");
        value = new GraphicalArray(new QRegExpValidator(regex, parent), parent);
      }
      else if(type == Protocol::Tags::type<Real>())            // Real option
      {
        QDoubleValidator * val = new QDoubleValidator(nullptr);
        val->setNotation(QDoubleValidator::ScientificNotation);
        value = new GraphicalArray(val, parent);
      }
      else if(type == Protocol::Tags::type<int>())              // int option
        value = new GraphicalArray(new QIntValidator(), parent);
      else if(type == Protocol::Tags::type<Uint>())             // Uint option
      {
        QIntValidator * val = new QIntValidator();
        val->setBottom(0);
        value = new GraphicalArray(val, parent);
      }
      else if(type == Protocol::Tags::type<std::string>())      // string option
        value = new GraphicalArray(nullptr, parent);
      else if(type == Protocol::Tags::type<URI>())              // URI option
        value = new GraphicalUriArray(parent);
      else
        throw CastingFailed(FromHere(), tag + ": Unknown type");

      value->setValue( QString(value_str.c_str()).split("@@") );
    }
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////

QString GraphicalValue::valueString() const
{
  QVariant value = this->value();

  if(value.type() == QVariant::StringList)
    return value.toStringList().join("@@");

  return value.toString();
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalValue::originalValue() const
{
  return m_originalValue;
}

//////////////////////////////////////////////////////////////////////////

QString GraphicalValue::originalValueString() const
{
  if(m_originalValue.type() == QVariant::StringList)
    return m_originalValue.toStringList().join("@@");

  return m_originalValue.toString();
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalValue::isModified() const
{
  return originalValueString() != valueString();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalValue::commit()
{
  m_originalValue = value();
  emit valueChanged();
}

//////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF
