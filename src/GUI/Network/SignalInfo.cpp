#include <QtCore>
#include <QtXml>

#include <string>

#include "Common/CF.hpp"
#include "GUI/Network/SignalInfo.hpp"

using namespace CF::Common;

// templace functions need to be defined inf the *same* namespace
// ("using namespace" is not sufficient!)
namespace CF {
namespace GUI {
namespace Network {

  SignalInfo::SignalInfo(const QString & key, const CPath & sender,
                         const CPath & receiver, bool isSignal)
    : m_isSignal(isSignal)
  {
    m_rootElt = m_doc.createElement(m_isSignal ? "Signal" : "Reply");
    m_paramElt = m_doc.createElement("Params");

    this->setKey(key);
    this->setSender(sender);
    this->setReceiver(receiver);

    m_rootElt.appendChild(m_paramElt);
    m_doc.appendChild(m_rootElt);
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::setKey(const QString & key)
  {
    if(key.isEmpty())
      throw BadValue(FromHere(), "The key is empty.");

    m_rootElt.setAttribute("key", key);
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::setSender(const CPath & sender)
  {
    m_rootElt.setAttribute("sender", sender.string().c_str());
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::setReceiver(const CPath & receiver)
  {
    m_rootElt.setAttribute("receiver", receiver.string().c_str());
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::setIsSignal(bool isSignal)
  {
    if(isSignal != m_isSignal)
    {
      m_isSignal = isSignal;
      m_rootElt.setTagName(m_isSignal ? "Signal" : "Reply");
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  QString SignalInfo::getKey() const
  {
    return m_rootElt.attribute("key");
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  CPath SignalInfo::getSender() const
  {
    return m_rootElt.attribute("sender").toStdString();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  CPath SignalInfo::getReceiver() const
  {
    return m_rootElt.attribute("receiver").toStdString();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  bool SignalInfo::isSignal() const
  {
    return m_rootElt.nodeName() == "Signal";
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  QDomDocument SignalInfo::getXmlDocument() const
  {
    return m_doc.cloneNode(true).toDocument();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  QString SignalInfo::getString() const
  {
    return m_doc.toString();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::convertToStdString(const QStringList & list1, QList<std::string> & list2)
  {
    QStringList::const_iterator it = list1.begin();

    list2.clear();

    while(it != list1.end())
    {
      list2.append(it->toStdString());
      it++;
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void SignalInfo::convertToStringList(const QList<std::string> & list1, QStringList & list2)
  {
    QList<std::string>::const_iterator it = list1.begin();

    list2.clear();

    while(it != list1.end())
    {
      list2 << it->c_str();
      it++;
    }
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  template<>
  QString SignalInfo::typeName(const bool &) const { return "bool"; }

  template<>
  QString SignalInfo::typeName(const std::string &) const { return "string"; }

  template<>
  QString SignalInfo::typeName(const int &) const { return "integer"; }

  template<>
  QString SignalInfo::typeName(const CF::Real &) const { return "real"; }

} // namespace Network
} // namespace GUI
} // namespace CF

//void SignalInfo::buildSignalDoc(QDomDocument & doc) const
//{
// QDomElement root;
// QDomElement params = doc.createElement("Params");
// QHash<QString, QString>::const_iterator itData = m_data.begin();
// QList<QDomElement>::const_iterator itVects = m_vectors.begin();
//
// if(m_isSignal)
//   root = doc.createElement("Signal");
// else
//   root = doc.createElement("Reply");
//
// while(doc.hasChildNodes())
//   doc.removeChild(doc.firstChild());
//
// // on error, QDomDocument should return a null node [1]
// QDomImplementation::setInvalidDataPolicy(QDomImplementation::ReturnNullNode);
//
// cf_assert( !m_type.isEmpty() );
// cf_assert( m_sender.is_absolute() );
// cf_assert( m_receiver.is_absolute() );
//
// root.setAttribute("type", m_type);
// root.setAttribute("sender", m_sender.string().c_str());
// root.setAttribute("receiver", m_receiver.string().c_str());
//
// while(itData != m_data.end())
// {
//   QDomElement elt = doc.createElement(itData.key());
//   QDomText value = doc.createTextNode(itData.value());
//
//   // if it was not a valid XML name, returns a null node because of [1]
//   cf_assert( !elt.isNull() );
//
//   elt.appendChild(value);
//
//   params.appendChild(elt);
//   itData++;
// }
//
// while(itVects != m_vectors.end())
// {
//   params.appendChild(*itVects);
//   itVects++;
// }
//
// root.appendChild(params);
// doc.appendChild(root);
//}
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void SignalInfo::buildSignalString(QString & str) const
//{
// QDomDocument doc;
//
// this->buildSignalDoc(doc);
//
// str = doc.toString();
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

