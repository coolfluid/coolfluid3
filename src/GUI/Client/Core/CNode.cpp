// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMenu>
#include <QPoint>
#include <QStringList>
#include <QVariant>
#include <QDebug>

#include <boost/filesystem/path.hpp>

#include <cstring>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/String/Conversion.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NCore.hpp"
#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NLink.hpp"
#include "GUI/Client/Core/NRoot.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "GUI/Client/Core/CNode.hpp"

#define ADD_ARRAY_TO_XML(type) { \
std::vector<type> data;\
boost::shared_ptr<OptionArrayT<type> > array;\
array = boost::dynamic_pointer_cast<OptionArrayT<type> >(array);\
        \
for( ; itList != list.end() ; itList++)\
  data.push_back(itList->toStdString());\
  \
p.add_array(it.key().toStdString(), data);\
}


using namespace CF::Common;
using namespace CF::Common::String;
using namespace CF::GUI::ClientCore;

bool NodeOption::operator==(const NodeOption & option)
{
  return m_paramAdv ^ option.m_paramAdv
      && m_paramName == option.m_paramName
      && m_paramType == option.m_paramType
      && m_paramValue == option.m_paramValue
      && m_paramDescr == option.m_paramDescr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNodeNotifier::CNodeNotifier(CNode * parent)
  : m_parent(parent)
{ }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNodeNotifier::notifyChildCountChanged()
{
  emit childCountChanged();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::CNode(const QString & name, const QString & componentType, CNode::Type type)
  : Component(name.toStdString()),
    m_type(type),
    m_notifier(new CNodeNotifier(this)),
    m_componentType(componentType)
{

  BUILD_COMPONENT;

  regist_signal("configure", "Update component options")->connect(boost::bind(&CNode::configure_reply, this, _1));
  regist_signal("list_signals", "Update component signals")->connect(boost::bind(&CNode::list_signals_reply, this, _1));

  m_property_list.add_property("originalComponentType", m_componentType.toStdString());

//  m_property_list.add_property("test", int(2));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::defineConfigProperties ( CF::Common::PropertyList& props )
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CNode::getComponentType() const
{
  return m_componentType;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Type CNode::getType() const
{
  return m_type;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setOptions(XmlNode & options)
{
  XmlParams p(options);

  if(p.option_map != nullptr)
  {
    // iterate through options
    XmlNode* node = p.option_map->first_node();
    for ( ; node != nullptr ; node = node->next_sibling(  ) )
    {
      bool advanced;
      XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );
      XmlAttr * descrAttr = node->first_attribute( XmlParams::tag_attr_descr() );
      XmlAttr * modeAttr = node->first_attribute( "mode" );

      advanced = modeAttr != nullptr && std::strcmp(modeAttr->value(), "adv") == 0;

      if ( keyAttr != nullptr )
      {
        const char * keyVal = keyAttr->value(); // option name

        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
        {
          XmlNode * type_node = node->first_node();

          if( type_node != nullptr)
          {
            const char * descrVal = (descrAttr != nullptr) ? descrAttr->value() : "";
            const char * typeVal = type_node->name(); // type name

            if(std::strcmp(typeVal, "bool") == 0)
              addOption<bool>(keyVal, descrVal, *type_node);
            else if(std::strcmp(typeVal, "integer") == 0)
              addOption<int>(keyVal, descrVal, *type_node);
            else if(std::strcmp(typeVal, "unsigned") == 0)
              addOption<CF::Uint>(keyVal, descrVal, *type_node);
            else if(std::strcmp(typeVal, "real") == 0)
              addOption<CF::Real>(keyVal, descrVal, *type_node);
            else if(std::strcmp(typeVal, "string") == 0)
              addOption<std::string>(keyVal, descrVal, *type_node);
            else if(std::strcmp(typeVal, "uri") == 0)
            {
              URI value;
              to_value(*type_node, value);
              m_property_list.add_option<OptionURI>(keyVal, descrVal, value);
              //addOption<boost::filesystem::path>(keyVal, descrVal, *type_node);
            }
            else
              throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
          }
        }
        else if(std::strcmp(node->name(), "array")  == 0)
        {
          XmlAttr * typeAttr= node->first_attribute( XmlParams::tag_attr_type() );

          if( typeAttr != nullptr)
          {
            const char * descrVal = (descrAttr != nullptr) ? descrAttr->value() : "";
            const char * typeVal = typeAttr->value(); // element type

            if(std::strcmp(typeVal, "bool") == 0)
            {
              std::vector<bool> data = p.get_array<bool>(keyVal);
              m_property_list.add_option< OptionArrayT<bool> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "integer") == 0)
            {
              std::vector<int> data = p.get_array<int>(keyVal);
              m_property_list.add_option< OptionArrayT<int> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "unsigned") == 0)
            {
              std::vector<unsigned int> data = p.get_array<unsigned int>(keyVal);
              m_property_list.add_option< OptionArrayT<unsigned int> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "real") == 0)
            {
              std::vector<CF::Real> data = p.get_array<CF::Real>(keyVal);
              m_property_list.add_option< OptionArrayT<CF::Real> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "string") == 0)
            {
              std::vector<std::string> data = p.get_array<std::string>(keyVal);
              m_property_list.add_option< OptionArrayT<std::string> >(keyVal, descrVal, data);
            }
            else if(std::strcmp(typeVal, "file") == 0)
            {
              std::vector<boost::filesystem::path> data;
              data = p.get_array<boost::filesystem::path>(keyVal);
              m_property_list.add_option< OptionArrayT<boost::filesystem::path> >(keyVal, descrVal, data);
            }
            else
              throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown array type");
          }

        }

        if(!advanced)
          m_property_list[keyVal].as_option().mark_basic();
      }
    }
  }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::setProperties(XmlNode & options)
{
  XmlParams p(options);

  if(p.property_map != nullptr)
  {
    // iterate through properties
    XmlNode* node = p.property_map->first_node();
    for ( ; node != nullptr ; node = node->next_sibling(  ) )
    {
      XmlAttr * keyAttr= node->first_attribute( XmlParams::tag_attr_key() );

      if ( keyAttr != nullptr )
      {
        const char * keyVal = keyAttr->value(); // option name

        if(std::strcmp(node->name(), XmlParams::tag_node_value())  == 0)
        {
          XmlNode * type_node = node->first_node();

          if( type_node != nullptr)
          {
            const char * typeVal = type_node->name(); // type name

            if(m_property_list.check(keyVal))
            {
              if(std::strcmp(typeVal, "bool") == 0)
                configure_property(keyVal, from_str<bool>(type_node->value()));
              else if(std::strcmp(typeVal, "integer") == 0)
                configure_property(keyVal, from_str<int>(type_node->value()));
              else if(std::strcmp(typeVal, "unsigned") == 0)
                configure_property(keyVal, from_str<CF::Uint>(type_node->value()));
              else if(std::strcmp(typeVal, "real") == 0)
                configure_property(keyVal, from_str<CF::Real>(type_node->value()));
              else if(std::strcmp(typeVal, "string") == 0)
                configure_property(keyVal, std::string(type_node->value()));
              else if(std::strcmp(typeVal, "uri") == 0)
                configure_property(keyVal, from_str<URI>(type_node->value()));
              else
                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());
            }
            else
            {
              const char * value = type_node->value();
              if(std::strcmp(typeVal, "bool") == 0)
                m_property_list.add_property(keyVal, from_str<bool>(value));
              else if(std::strcmp(typeVal, "integer") == 0)
                m_property_list.add_property(keyVal, from_str<int>(value));
              else if(std::strcmp(typeVal, "unsigned") == 0)
                m_property_list.add_property(keyVal, from_str<CF::Uint>(value));
              else if(std::strcmp(typeVal, "real") == 0)
                m_property_list.add_property(keyVal, from_str<CF::Real>(value));
              else if(std::strcmp(typeVal, "string") == 0)
                m_property_list.add_property(keyVal, std::string(value));
              else if(std::strcmp(typeVal, "uri") == 0)
                m_property_list.add_property(keyVal, from_str<URI>(value));
              else
                throw ShouldNotBeHere(FromHere(), std::string(typeVal) + ": Unknown type parent is " + node->name());

            }
          }
        }
      }
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::modifyOptions(const QMap<QString, QString> & options)
{
  QMap<QString, QString>::const_iterator it = options.begin();

  if(isClientComponent())
  {
    for ( ; it != options.end() ; it++)
    {
//			if(m_options.contains(it.key()))
//				m_options[it.key()].m_paramValue = it.value();
    }
  }
  else
  {
    boost::shared_ptr<XmlDoc> docnode = XmlOps::create_doc();
    XmlNode * rootNode = XmlOps::goto_doc_node(*docnode.get());
    XmlNode * signalNode = XmlOps::add_signal_frame(*rootNode, "configure", full_path(), full_path(), true);
    XmlParams p(*signalNode);
    bool valid = true;

    for ( ; it != options.end() ; it++)
    {
      Property * prop = &m_property_list[it.key().toStdString()].as_option();

      if( prop != nullptr && strcmp( prop->tag() , "array" ) )
      {
        std::string name = it.key().toStdString();
        QString value = it.value();

        if(prop->type() == "bool")
          p.add_option(name, QVariant(value).toBool());
        else if(prop->type() == "integer")
          p.add_option(name, value.toInt());
        else if(prop->type() == "unsigned")
          p.add_option(name, value.toUInt());
        else if(prop->type() == "real")
          p.add_option(name, value.toDouble());
        else if(prop->type() == "string")
          p.add_option(name, value.toStdString());
        else if(prop->type() == "file")
          p.add_option(name, boost::filesystem::path(value.toStdString()));
        else
          throw ValueNotFound(FromHere(), prop->type() + ": Unknown type for option " + name );
      }
      else if( prop != nullptr && !strcmp ( prop->tag() , "array" ))
      {
        OptionArray * optArray;
        QStringList list = it.value().split(":");
        QStringList::iterator itList = list.begin();

        optArray = static_cast<OptionArray*>(prop);

        cf_assert(optArray != nullptr);

        const char * elemType = optArray->elem_type();

        if(std::strcmp(elemType, "file") == 0)
          ADD_ARRAY_TO_XML(boost::filesystem::path)
        else
          throw ValueNotFound(FromHere(), std::string(elemType) + ": Unknown type for option array " + optArray->name());
      }
    }

    if(valid)
      ClientRoot::core()->sendSignal(*docnode.get());
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXml(CF::Common::XmlNode & node)
{
  QMap<NLink::Ptr, CPath> linkTargets;
  QMap<NLink::Ptr, CPath>::iterator it;
  CNode::Ptr rootNode;

  rootNode = createFromXmlRec(node, linkTargets);

  it = linkTargets.begin();

  for( ; it != linkTargets.end() ; it++)
    it.key()->setTargetPath(it.value());

  return rootNode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::getNode(CF::Uint index)
{
  Component::Ptr compo = shared_from_this();
  CF::Uint i;

  if(checkType(CNode::ROOT_NODE))
    compo = this->convertTo<NRoot>()->root();

  ComponentIterator<CNode> it = compo->begin<CNode>();

  cf_assert(index < compo->get_child_count());

  for(i = 0 ; i < index ; i++)
    it++;

  return it.get();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::connectNotifier(QObject * reciever, const char * signal, const char * slot)
{
  QObject::connect(m_notifier, signal, reciever, slot);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNodeNotifier * CNode::getNotifier() const
{
  return m_notifier;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::listChildPaths(QStringList & list, bool recursive, bool clientNodes) const
{
  ComponentIterator<const CNode> it = this->begin<const CNode>();
  ComponentIterator<const CNode> itEnd = this->end<const CNode>();

  if(this->checkType(ROOT_NODE))
  {
    CRoot::ConstPtr root = this->convertTo<const NRoot>()->root();
    it = root->begin<const CNode>();
    itEnd = root->end<const CNode>();

    if(it->get_child_count() > 0)
      list << QString(root->full_path().string().c_str()) /*+ '/'*/;
    else
      list << root->full_path().string().c_str();
  }

  for( ; it != itEnd ; it++)
  {
    if(!it->isClientComponent() || clientNodes)
    {
      if(it->get_child_count() > 0)
        list << QString(it->full_path().string().c_str()) /*+ '/'*/;
      else
        list << it->full_path().string().c_str();
    }

    if(recursive)
      it->listChildPaths(list, recursive, clientNodes);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::addNode(CNode::Ptr node)
{
  try
  {
    if(checkType(ROOT_NODE))
      ((NRoot *)this)->root()->add_component(node);
    else
      this->add_component(node);

    m_notifier->notifyChildCountChanged();
  }
  catch(CF::Common::ValueExists & ve)
  {
    throw;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::removeNode(const QString & nodeName)
{
  try
  {
    if(checkType(ROOT_NODE))
      ((NRoot *)this)->root()->remove_component(nodeName.toStdString());
    else
      this->remove_component(nodeName.toStdString());

    m_notifier->notifyChildCountChanged();
  }
  catch(CF::Common::ValueNotFound & ve)
  {
    throw;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::configure_reply(CF::Common::XmlNode & node)
{
  ClientRoot::tree()->optionsChanged(this->full_path());
  ClientRoot::log()->addMessage(QString("Node \"%1\" options updated.").arg(full_path().string().c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getOptions(QList<NodeOption> & options) const
{
  PropertyList::PropertyStorage_t::const_iterator it = m_property_list.m_properties.begin();

  options.clear();

  for( ; it != m_property_list.m_properties.end() ; it++)
  {
    if(it->second->is_option())
    {
      Option::Ptr opt = boost::dynamic_pointer_cast<Option>(it->second);
      bool success = true;
      NodeOption nodeOpt;
      //OptionType::Type optionType = OptionType::Convert::to_enum(opt->type());

      nodeOpt.m_paramAdv= !opt->has_tag("basic");
      nodeOpt.m_paramName = it->first.c_str();
      nodeOpt.m_paramValue = opt->value_str().c_str();
      nodeOpt.m_paramDescr = opt->description().c_str();

      if(opt->has_restricted_list())
      {
        const std::vector<boost::any> & vect = opt->restricted_list();

        nodeOpt.m_paramType = OptionType::TYPE_LIST;

        try
        {
          if(opt->type().compare(XmlTag<std::string>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<std::string>(vect);
          else if(opt->type().compare(XmlTag<CF::Uint>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<CF::Uint>(vect);
          else if(opt->type().compare(XmlTag<CF::Real>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<CF::Real>(vect);
          else if(opt->type().compare(XmlTag<int>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<int>(vect);
          else if(opt->type().compare(XmlTag<bool>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<bool>(vect);
          else if(opt->type().compare(XmlTag<URI>::type()) == 0)
            nodeOpt.m_paramRestrValues = this->vectToStringList<URI>(vect);
          else
            ClientRoot::log()->addError(QString("%1: unknown type").arg(opt->type().c_str()));
        }
        catch(boost::bad_any_cast & ex)
        {
          QString msg("Could not build restricted list: %1");
          ClientRoot::log()->addException(msg.arg(opt->restricted_list().size()));
        }
      }
      else
      {
        if(std::strcmp(opt->tag(), "array") != 0)
          nodeOpt.m_paramType = OptionType::Convert::to_enum(opt->type());
        else
        {
          boost::shared_ptr<OptionArray> optArray;
          optArray = boost::dynamic_pointer_cast<OptionArray>(opt);

          if(std::strcmp(optArray->elem_type(), "file") == 0)
            nodeOpt.m_paramType = OptionType::TYPE_FILES;
          else
          {
            success = false;
            ClientRoot::log()->addError(QString("Unable to process %1 option array")
                                        .arg(optArray->elem_type()));
          }
        }
      }

      if(success)
        options.append(nodeOpt);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getProperties(QMap<QString, QString> & props) const
{
  PropertyList::PropertyStorage_t::const_iterator it = m_property_list.m_properties.begin();

  props.clear();

  for( ; it != m_property_list.m_properties.end() ; it++)
  {
    if(!it->second->is_option())
      props[ it->first.c_str() ] = it->second->value_str().c_str();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::getActions(QList<ActionInfo> & actions) const
{
  actions = m_actionSigs;

  QStringList::const_iterator it = m_localSignals.begin();

  for( ; it != m_localSignals.end() ; it++)
  {

    if(m_signals.find(it->toStdString()) != m_signals.end())
    {
      ActionInfo ai;
      const Signal & sig = m_signals.find(it->toStdString())->second;

      ai.m_name = it->toStdString().c_str();
      ai.m_description = sig.m_description.c_str();
      ai.m_readableName = sig.m_readable_name.c_str();
      ai.m_signature = sig.m_signature;
      ai.m_is_local = true;

      actions.append(ai);
    }
    else
      ClientRoot::log()->addError(*it + ": local signal not found");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::fetchSignals()
{
  if(m_fetchingManager.get() == nullptr)
  {
    ClientRoot::log()->addMessage("Fetching actions...");
    listChildPaths(m_fetchingChildren, true, false);

    m_fetchingManager = boost::dynamic_pointer_cast<CNode>(shared_from_this());
    fetchSignals();
  }
  else
  {
    CPath path;

    if(m_type == ROOT_NODE)
      path = convertTo<NRoot>()->root()->full_path();
    else
      path = full_path();

    boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
    XmlOps::add_signal_frame(*XmlOps::goto_doc_node(*doc.get()),
                             "list_signals", path,
                             path, false);

    ClientRoot::core()->sendSignal(*doc);
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::signalsFetched(CNode::Ptr notifier)
{
  int index;
  QString notifier_path;

  if(notifier->checkType(ROOT_NODE))
    notifier_path = notifier->convertTo<NRoot>()->root()->full_path().string().c_str();
  else
    notifier_path = notifier->full_path().string().c_str();

  index = m_fetchingChildren.indexOf(notifier_path);

  index++;

  notifier->m_fetchingManager = CNode::Ptr();

  if(index == -1)
    ClientRoot::log()->addError(QString("Received unexpected actions fetched notification from %1").arg(notifier_path));
  else
  {
    if(index == m_fetchingChildren.size())
    {
      m_fetchingChildren.clear();
      ClientRoot::log()->addMessage("Actions successfully fetched.");
    }
    else
    {
      QString path = m_fetchingChildren.at(index);

      CRoot::Ptr root = m_type == ROOT_NODE ? convertTo<NRoot>()->root() : boost::dynamic_pointer_cast<CRoot>(m_root.lock());

      CNode::Ptr node;

      if(m_type == ROOT_NODE && root->full_path().string() == path.toStdString())
        node = boost::dynamic_pointer_cast<CNode>(shared_from_this());
      else
        node = root->access_component<CNode>(path.toStdString());

      node->m_fetchingManager = boost::dynamic_pointer_cast<CNode>(shared_from_this());
      node->fetchSignals();
    }

  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CNode::list_signals_reply( XmlNode & node )
{
  XmlNode * map = node.first_node();

  m_actionSigs.clear();

  while(map != nullptr)
  {
    ActionInfo si;
    XmlAttr * key_attr = map->first_attribute( XmlParams::tag_attr_key() );
    XmlAttr * desc_attr = map->first_attribute( XmlParams::tag_attr_descr() );
    XmlAttr * name_attr = map->first_attribute( "name" );

    cf_assert( key_attr != nullptr );
    cf_assert( key_attr->value_size() > 0 );

    si.m_name = key_attr->value();
    si.m_readableName = name_attr != nullptr ? name_attr->value() : "";
    si.m_description = desc_attr != nullptr ? desc_attr->value() : "";
    si.m_signature = XmlSignature(*map);
    si.m_is_local = false;

    m_actionSigs.append(si);

    map = map->next_sibling();
  }

  if(m_fetchingManager.get() != nullptr)
    m_fetchingManager->signalsFetched(boost::dynamic_pointer_cast<CNode>(shared_from_this()));
  else
    ClientRoot::log()->addMessage("Received actions !");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr CNode::createFromXmlRec(XmlNode & node, QMap<NLink::Ptr, CPath> & linkTargets)
{
  XmlAttr * typeAttr = node.first_attribute("atype");
  XmlAttr * nameAttr = node.first_attribute("name");

  cf_assert(typeAttr != nullptr);
  cf_assert(nameAttr != nullptr);

  char * typeName = typeAttr->value();
  char * nodeName = nameAttr->value();
  XmlNode * child = node.first_node();

  cf_assert(typeName != nullptr);
  cf_assert(nodeName != nullptr);
  cf_assert(std::strlen(typeName) > 0);
  cf_assert(std::strlen(nodeName) > 0);

  CNode::Ptr rootNode;

  if(std::strcmp(typeName, "CCore") == 0 || std::strcmp(typeName, "CSimulator") == 0)
    return rootNode;

  if(std::strcmp(typeName, "CLink") == 0)
  {
    NLink::Ptr link = boost::shared_ptr<NLink>(new NLink(nodeName));
    rootNode = link;
    linkTargets[link] = node.value();
  }
  else if(std::strcmp(typeName, "CRoot") == 0)
    rootNode = boost::shared_ptr<NRoot>(new NRoot(nodeName));
  else
    rootNode = boost::shared_ptr<NGeneric>(new NGeneric(nodeName, typeName));


  while(child != nullptr)
  {
    try
    {
      if(std::strcmp(child->name(), XmlParams::tag_node_map()) == 0)
      {
        rootNode->setOptions(node);
        rootNode->setProperties(node);
      }
      else
      {
        CNode::Ptr node = createFromXmlRec(*child, linkTargets);

        if(node.get() != nullptr)
        {
          if(rootNode->checkType(ROOT_NODE))
            rootNode->convertTo<NRoot>()->root()->add_component(node);
          else
            rootNode->add_component(node);
        }
      }
    }
    catch (ShouldNotBeHere & snbh)
    {
      ClientRoot::log()->addException(snbh.msg().c_str());
    }

    child = child->next_sibling();
  }

  return rootNode;
}

#undef ADD_ARRAY_TO_XML
