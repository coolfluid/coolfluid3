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

GraphicalValue * GraphicalValue::createFromXml(const XmlNode & node,
                                               QWidget * parent)
{
//  GraphicalValue * value = nullptr;

  Option::Ptr option = CNode::makeOption(node);

  return createFromOption(option, parent);

//  XmlAttr * keyAttr = node.first_attribute( XmlParams::tag_attr_key() );

//  if(keyAttr == nullptr)
//    throw XmlError(FromHere(), "Key attribute not found");

//  const char * keyVal = keyAttr->value(); // option name

//  if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
//  {
//    XmlNode * type_node = node->first_node();

//    if( type_node != nullptr)
//    {
//      const char * descrVal = (descrAttr != nullptr) ? descrAttr->value() : "";
//      const char * typeVal = type_node->name(); // type name

//      if(std::strcmp(typeVal, "bool") == 0)
//        addOption<bool>(keyVal, descrVal, *type_node);
//      else if(std::strcmp(typeVal, "integer") == 0)
//        addOption<int>(keyVal, descrVal, *type_node);
//      else if(std::strcmp(typeVal, "unsigned") == 0)
//        addOption<CF::Uint>(keyVal, descrVal, *type_node);
//      else if(std::strcmp(typeVal, "real") == 0)
//        addOption<CF::Real>(keyVal, descrVal, *type_node);
//      else if(std::strcmp(typeVal, "string") == 0)
//        addOption<std::string>(keyVal, descrVal, *type_node);
//      else if(std::strcmp(typeVal, "uri") == 0)
//      {
//        URI value;
//        Option::Ptr option;
//        OptionURI::Ptr opt;
//        //XmlAttr * attr = node->first_attribute(XmlParams::tag_attr_protocol());
//        to_value(*type_node, value);
////              opt = m_property_list.add_option<OptionURI>(keyVal, descrVal, value);

////              for( ; attr != nullptr ; attr = attr->next_attribute(XmlParams::tag_attr_protocol()))
////              {
////                opt->supported_protocol(attr->value());
////              }
//      }
//      else
//        throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
//    }
//  }
//  else if(std::strcmp(node->name(), "array")  == 0)
//  {
//    XmlAttr * typeAttr= node->first_attribute( XmlParams::tag_attr_type() );

//    if( typeAttr != nullptr)
//    {
//      const char * descrVal = (descrAttr != nullptr) ? descrAttr->value() : "";
//      const char * typeVal = typeAttr->value(); // element type

//      if(std::strcmp(typeVal, "bool") == 0)
//      {
//        std::vector<bool> data = p.get_array<bool>(keyVal);
//        m_property_list.add_option< OptionArrayT<bool> >(keyVal, descrVal, data);
//      }
//      else if(std::strcmp(typeVal, "integer") == 0)
//      {
//        std::vector<int> data = p.get_array<int>(keyVal);
//        m_property_list.add_option< OptionArrayT<int> >(keyVal, descrVal, data);
//      }
//      else if(std::strcmp(typeVal, "unsigned") == 0)
//      {
//        std::vector<unsigned int> data = p.get_array<unsigned int>(keyVal);
//        m_property_list.add_option< OptionArrayT<unsigned int> >(keyVal, descrVal, data);
//      }
//      else if(std::strcmp(typeVal, "real") == 0)
//      {
//        std::vector<CF::Real> data = p.get_array<CF::Real>(keyVal);
//        m_property_list.add_option< OptionArrayT<CF::Real> >(keyVal, descrVal, data);
//      }
//      else if(std::strcmp(typeVal, "string") == 0)
//      {
//        std::vector<std::string> data = p.get_array<std::string>(keyVal);
//        m_property_list.add_option< OptionArrayT<std::string> >(keyVal, descrVal, data);
//      }
//      else if(std::strcmp(typeVal, "file") == 0)
//      {
//        std::vector<boost::filesystem::path> data;
//        data = p.get_array<boost::filesystem::path>(keyVal);
//        m_property_list.add_option< OptionArrayT<boost::filesystem::path> >(keyVal, descrVal, data);
//      }
//      else
//        throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown array type");
//    }

//  }


//  if(option->has_restricted_list())
//    value = new GraphicalRestrictedList(option, parent);
//  else if(tag.compare("array") != 0)
//  {
//    if(type.compare(XmlTag<bool>::type()) == 0)               // bool option
//      value = new GraphicalBool(option, parent);
//    else if(tag.compare(XmlTag<CF::Real>::type()) == 0)      // Real option
//      value = new GraphicalDouble(option, parent);
//    else if(tag.compare(XmlTag<int>::type()) == 0)           // int option
//      value = new GraphicalInt(false, option, parent);
//    else if(tag.compare(XmlTag<CF::Uint>::type()) == 0)      // Uint option
//      value = new GraphicalInt(true, option, parent);
//    else if(tag.compare(XmlTag<std::string>::type()) == 0)   // string option
//      value = new GraphicalString(option, parent);
//    else if(tag.compare(XmlTag<URI>::type()) == 0)           // URI option
//      value = new GraphicalUri(boost::dynamic_pointer_cast<OptionURI const>(option), parent);
//    else
//      throw CastingFailed(FromHere(), tag + ": Unknown type");
//  }
//  else
//  {
//    OptionArray::ConstPtr array = boost::dynamic_pointer_cast<OptionArray const>(option);

//    tag = array->elem_type();

//    if(tag.compare(XmlTag<bool>::type()) == 0)               // bool option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<CF::Real>::type()) == 0)      // Real option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<int>::type()) == 0)           // int option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<CF::Uint>::type()) == 0)      // Uint option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<std::string>::type()) == 0)   // string option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<URI>::type()) == 0)           // URI option
//      throw CastingFailed(FromHere(), tag + ": This type is not supported yet for arrays");
//    else if(tag.compare(XmlTag<boost::filesystem::path>::type()) == 0)           // path option
//      value = new GraphicalUrlArray(parent);
//    else
//      throw CastingFailed(FromHere(), tag + ": Unknown type");
//  }

//  return value;
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
