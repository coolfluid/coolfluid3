// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include <QHBoxLayout>
#include <QRegExpValidator>

#include "math/Consts.hpp"

#include "common/OptionArray.hpp"

#include "common/XML/Protocol.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/CNode.hpp"

#include "ui/graphics/GraphicalArray.hpp"
#include "ui/graphics/GraphicalArrayRestrictedList.hpp"
#include "ui/graphics/GraphicalBool.hpp"
#include "ui/graphics/GraphicalDouble.hpp"
#include "ui/graphics/GraphicalInt.hpp"
#include "ui/graphics/GraphicalRestrictedList.hpp"
#include "ui/graphics/GraphicalString.hpp"
#include "ui/graphics/GraphicalUri.hpp"
#include "ui/graphics/GraphicalUriArray.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

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

GraphicalValue * GraphicalValue::create_from_option(boost::shared_ptr< Option > option,
                                                  QWidget * parent )
{
  GraphicalValue * value = nullptr;

  if(option.get() == nullptr)
    return value;

  std::string type(option->type());

  if(!boost::starts_with(type, "array"))
  {
    if(option->has_restricted_list())
      value = new GraphicalRestrictedList(option, parent);
    else
    {

      if(type == common::class_name<bool>())               // bool option
        value = new GraphicalBool(option->value<bool>(), parent);
      else if(type == common::class_name<Real>())          // Real option
        value = new GraphicalDouble(option->value<Real>(), parent);
      else if(type == common::class_name<int>())           // int option
        value = new GraphicalInt(false, option->value<int>(), parent);
      else if(type == common::class_name<Uint>())          // Uint option
        value = new GraphicalInt(true, option->value<Uint>(), parent);
      else if(type == common::class_name<std::string>())   // string option
        value = new GraphicalString(option->value<std::string>().c_str(), parent);
      else if(type == common::class_name<URI>())           // URI option
        value = new GraphicalUri(boost::dynamic_pointer_cast<OptionURI>(option), parent);
      else
        throw CastingFailed(FromHere(), type + ": Unknown type");
    }
  }
  else
  {
    if(option->has_restricted_list())
      value = new GraphicalArrayRestrictedList(option, parent);
    else
    {
      std::string value_str( option->value_str() );
      std::string type( option->element_type() );
      QString sep( option->separator().c_str() );

      if(type == common::class_name<bool>())                 // bool option
      {
        QRegExp regex("(true)|(false)|(1)|(0)|(on)|(off)");
        value = new GraphicalArray(new QRegExpValidator(regex, parent), sep, parent);
      }
      else if(type == common::class_name<Real>())            // Real option
      {
        QDoubleValidator * val = new QDoubleValidator(nullptr);
        val->setNotation(QDoubleValidator::ScientificNotation);
        value = new GraphicalArray(val, sep, parent);
      }
      else if(type == common::class_name<int>())              // int option
        value = new GraphicalArray(new QIntValidator(), sep, parent);
      else if(type == common::class_name<Uint>())             // Uint option
      {
        QIntValidator * val = new QIntValidator();
        val->setBottom(0);
        value = new GraphicalArray(val, sep, parent);
      }
      else if(type == common::class_name<std::string>())      // string option
        value = new GraphicalArray(nullptr,sep,  parent);
      else if(type == common::class_name<URI>())              // URI option
        value = new GraphicalUriArray(sep, parent);
      else
        throw CastingFailed(FromHere(), type + ": Unknown type");

      value->set_value( QString(value_str.c_str()).split(option->separator().c_str()) );
    }
  }
  return value;
}

//////////////////////////////////////////////////////////////////////////

QString GraphicalValue::value_string() const
{
  QVariant value = this->value();

  if(value.type() == QVariant::StringList)
    return value.toStringList().join( m_separator );

  return value.toString();
}

//////////////////////////////////////////////////////////////////////////

QVariant GraphicalValue::original_value() const
{
  return m_original_value;
}

//////////////////////////////////////////////////////////////////////////

QString GraphicalValue::original_value_string() const
{
  if(m_original_value.type() == QVariant::StringList)
    return m_original_value.toStringList().join( m_separator );

  return m_original_value.toString();
}

//////////////////////////////////////////////////////////////////////////

bool GraphicalValue::is_modified() const
{
  return original_value_string() != value_string();
}

//////////////////////////////////////////////////////////////////////////

void GraphicalValue::commit()
{
  m_original_value = value();
  emit value_changed();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
