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
  QStringList restrList;
  QStringList valList;

  m_listView = new QListView(parent);
  m_model = new QStandardItemModel(parent);

  m_listView->setModel(m_model);

  m_layout->addWidget(m_listView);

  connect(m_model, SIGNAL(itemChanged(QStandardItem*)),
          this, SLOT(itemChanged(QStandardItem*)));

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
    {
      vectToStringList<bool>(vect, restrList);
      anyToStringList<bool>(opt->value(), valList);
    }
    else if(type.compare(XmlTag<CF::Real>::type()) == 0)     // Real option
    {
      vectToStringList<CF::Real>(vect, restrList);
      anyToStringList<CF::Real>(opt->value(), valList);
    }
    else if(type.compare(XmlTag<int>::type()) == 0)          // int option
    {
      vectToStringList<int>(vect, restrList);
      anyToStringList<int>(opt->value(), valList);
    }
    else if(type.compare(XmlTag<CF::Uint>::type()) == 0)     // Uint option
    {
      vectToStringList<CF::Uint>(vect, restrList);
      anyToStringList<CF::Uint>(opt->value(), valList);
    }
    else if(type.compare(XmlTag<std::string>::type()) == 0)  // string option
    {
      vectToStringList<std::string>(vect, restrList);
      anyToStringList<std::string>(opt->value(), valList);
    }
    else if(type.compare(XmlTag<URI>::type()) == 0)          // URI option
    {
      vectToStringList<URI>(vect, restrList);
      anyToStringList<URI>(opt->value(), valList);
    }
    else
      throw CastingFailed(FromHere(), type + ": Unknown type");

    setRestrictedList(restrList);
    setValue(valList);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalArrayRestrictedList::~GraphicalArrayRestrictedList()
{
  delete m_listView;
  delete m_model;
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
      QList<QStandardItem*>::const_iterator itItems = items.begin();

      for( ; itItems != items.end() ; itItems++)
        (*itItems)->setCheckState(Qt::Checked);
    }

    m_originalValue = value;

  }

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalArrayRestrictedList::value() const
{
  QStringList values;
  int rowCount = m_model->rowCount();

  for(int i = 0 ; i != rowCount ; i++)
  {
    QStandardItem *item = m_model->item(i);

    if(item != nullptr && item->checkState() == Qt::Checked)
      values << item->text();
  }

  return values;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::itemChanged(QStandardItem * item)
{
  emit valueChanged();
}
