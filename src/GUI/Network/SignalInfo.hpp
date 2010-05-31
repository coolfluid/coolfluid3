#ifndef CF_GUI_Network_SignalInfo_hpp
#define CF_GUI_Network_SignalInfo_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDomDocument>
#include <QHash>
#include <QList>
#include <QVariant>
#include <boost/any.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/CPath.hpp"
#include "Common/StringOps.hpp"

#include "GUI/Network/NetworkAPI.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

 ////////////////////////////////////////////////////////////////////////////

  /// @todo this class should disappear !!!

  class Network_API XMLParams
  {
    public:

      XMLParams(const std::string & frame)
      {
        m_doc.setContent(QString(frame.c_str()));
      }

      template<typename TYPE>
      TYPE value(const QString & key) const
      {
        QDomElement elt = this->getElement(key);

        if(elt.isNull())
          throw CF::Common::BadValue(FromHere(), "Unknown parameter");

        return CF::Common::StringOps::from_str<TYPE>(elt.firstChild().toText().nodeValue().toStdString());
      }

      template<typename TYPE>
      QList<TYPE> array(const QString & key)
      {
        QDomElement elt = this->getElement(key);
        QDomElement item = elt.firstChildElement();
        QList<TYPE> list;

        if(elt.isNull())
          throw CF::Common::BadValue(FromHere(), "Unknown array");

        if(elt.nodeName() != "array")
          throw CF::Common::BadValue(FromHere(), QString("%1 is not an array").arg(key).toStdString());

        while(!item.isNull())
        {
          list.append(CF::Common::StringOps::from_str<TYPE>(item.firstChild().toText().nodeValue().toStdString()));
          item = item.nextSiblingElement();
        }

        return list;
      }

      CF::Common::CPath getSender() const
      {
        return m_doc.firstChildElement().attribute("sender").toStdString();
      }

      CF::Common::CPath getReceiver() const
      {
        return m_doc.firstChildElement().attribute("receiver").toStdString();
      }

      bool isSignal() const
      {
        return m_doc.firstChildElement().nodeName() == "Signal";
      }

    private:

      QDomDocument m_doc;

      QDomElement getElement(const QString & key) const
      {
        QDomElement params = m_doc.firstChildElement().firstChildElement("Params");
        QDomElement elt = params.firstChildElement();
        bool found = false;

        while(!elt.isNull() && !found)
        {
          found = elt.attribute("key") == key;

          if(!found)
            elt = elt.nextSiblingElement();
        }

        if(!found)
          elt.clear();

        return elt;
      }
  };

  ////////////////////////////////////////////////////////////////////////////

 class  Network_API SignalInfo
 {
  public:

   SignalInfo(const QString & key, const CF::Common::CPath & sender,
              const CF::Common::CPath & receiver, bool isSignal);

   void setKey(const QString & key);

   void setSender(const CF::Common::CPath & sender);

   void setReceiver(const CF::Common::CPath & receiver);

   void setIsSignal(bool isSignal);

   template<typename TYPE>
   void setParam(const QString & key, const TYPE & value);

   template<typename TYPE>
   void setArray(const QString & key, const QList<TYPE> & list);

   QString getKey() const;

   CF::Common::CPath getSender() const;

   CF::Common::CPath getReceiver() const;

   bool isSignal() const;

   QDomDocument getXmlDocument() const;

   QString getString() const;

   static void convertToStdString(const QStringList & list1, QList<std::string> & list2);

   static void convertToStringList(const QList<std::string> & list1, QStringList & list2);


//   void buildSignalDoc(QDomDocument & doc) const;
//
//   void buildSignalString(QString & str) const;
//
//   template<typename TYPE>
//   void addVector(const QString & key, const QString & type,
//                  const QList<TYPE> & vect);

 private:

   QDomDocument m_doc;

   QDomElement m_rootElt;

   QDomElement m_paramElt;

   QList<QDomElement> m_vectors;

   QString m_type;

   bool m_isSignal;

   CF::Common::CPath m_sender;

   CF::Common::CPath m_receiver;

   QHash< QString, QString > m_data;

   template<typename TYPE>
   QString typeName(const TYPE & data) const;

 }; // class SignalInfo

 ////////////////////////////////////////////////////////////////////////////

 template<typename TYPE>
 void SignalInfo::setParam(const QString & key, const TYPE & value)
 {
   QString typeStr = this->typeName(value);
   QDomElement elt = m_doc.createElement(typeStr);
   QDomText txt = m_doc.createTextNode(CF::Common::StringOps::to_str(value).c_str());

   elt.setAttribute("key", key);

   elt.appendChild(txt);
   m_paramElt.appendChild(elt);
 }

 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 template<typename TYPE>
 void SignalInfo::setArray(const QString & key, const QList<TYPE> & list)
 {
   QDomElement elt = m_doc.createElement("array");
   typename QList<TYPE>::const_iterator it = list.begin();

   elt.setAttribute("key", key);
   elt.setAttribute("type", this->typeName(TYPE()));
   elt.setAttribute("size", list.size());

   while(it != list.end())
   {
     QDomElement item = m_doc.createElement("e");

     item.appendChild(m_doc.createTextNode(CF::Common::StringOps::to_str(*it).c_str()));
     elt.appendChild(item);

     it++;
   }

   m_paramElt.appendChild(elt);
 }

 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// template<typename TYPE>
// void SignalInfo::addVector(const QString & key, const QString & type,
//                            const QList<TYPE> & vect)
// {
//   QDomElement vectElt = m_doc.createElement("vector");
//   typename QList<TYPE>::const_iterator it = vect.begin();
//
//   cf_assert(!key.isEmpty());
//   cf_assert(!type.isEmpty());
//
//   vectElt.setAttribute("key", key);
//   vectElt.setAttribute("type", type);
//   vectElt.setAttribute("size", vect.size());
//
//   while(it != vect.end())
//   {
//     QDomElement elt = m_doc.createElement("e");
//
//     elt.appendChild(m_doc.createTextNode(QVariant(*it).toString()));
//     vectElt.appendChild(elt);
//
//     it++;
//   }
//
//   m_vectors.push_back(vectElt);
// }

 ////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_SignalInfo_hpp
