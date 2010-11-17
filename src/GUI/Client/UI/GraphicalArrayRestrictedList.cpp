// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QListView>
#include <QStandardItemModel>
#include <QDebug>
#include <QHBoxLayout>

#include "Common/OptionArray.hpp"
#include "Common/URI.hpp"

#include "GUI/Client/UI/GraphicalArrayRestrictedList.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientUI;

GraphicalArrayRestrictedList::GraphicalArrayRestrictedList(Option::ConstPtr opt,
                                                           QWidget * parent)
{
  QStringList list;

  m_listView = new QListView(parent);
  m_model = new QStandardItemModel(parent);

  m_listView->setModel(m_model);

  m_layout->addWidget(m_listView);

  if(opt.get() != nullptr && std::strcmp(opt->tag(), "array") == 0 &&
     opt->has_restricted_list())
  {
    OptionArray::ConstPtr array;
    std::string type;

    try
    {
      array = boost::dynamic_pointer_cast<const OptionArray>(opt);
    }
    catch(boost::bad_any_cast & bac)
    {
      throw CastingFailed(FromHere(), "Unable to cast to OptionArray");
    }

    const std::vector<boost::any> & vect = array->restricted_list();

    type = array->elem_type();

    if(type.compare(XmlTag<bool>::type()) == 0)              // bool option
      vectToStringList<bool>(vect, list);
    else if(type.compare(XmlTag<CF::Real>::type()) == 0)     // Real option
      vectToStringList<CF::Real>(vect, list);
    else if(type.compare(XmlTag<int>::type()) == 0)          // int option
      vectToStringList<int>(vect, list);
    else if(type.compare(XmlTag<CF::Uint>::type()) == 0)     // Uint option
      vectToStringList<CF::Uint>(vect, list);
    else if(type.compare(XmlTag<std::string>::type()) == 0)  // string option
      vectToStringList<std::string>(vect, list);
    else if(type.compare(XmlTag<URI>::type()) == 0)          // URI option
      vectToStringList<URI>(vect, list);
    else
      throw CastingFailed(FromHere(), type + ": Unknown type");

    setRestrictedList(list);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalArrayRestrictedList::~GraphicalArrayRestrictedList()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::setRestrictedList(const QStringList & list)
{
  QStringList::const_iterator it = list.begin();

  m_model->clear();

  for( ; it != list.end() ; it++)
  {
    QStandardItem * item = new QStandardItem(*it);
    item->setCheckable(true);
    m_model->appendRow(item);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalArrayRestrictedList::setValue(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::StringList)
  {
    QStringList list = value.toStringList();
    QStringList::const_iterator it = list.begin();

    for( ; it != list.end() ; it++)
    {
      QList<QStandardItem*> items = m_model->findItems(*it);

      if(items.size() == 1)
      {

      }
    }

  }

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalArrayRestrictedList::value() const
{
  return QVariant();
}

