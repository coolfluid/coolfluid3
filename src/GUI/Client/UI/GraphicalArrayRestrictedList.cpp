// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QListView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringListModel>

#include "Common/OptionArray.hpp"
#include "Common/URI.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Client/UI/GraphicalArrayRestrictedList.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

GraphicalArrayRestrictedList::GraphicalArrayRestrictedList(Option::ConstPtr opt,
                                                           QWidget * parent)
{
  QStringList restrList;
  QStringList valList;

  m_allowedListView = new QListView(parent);
  m_allowedModel = new QStringListModel(parent);
  m_selectedListView = new QListView(parent);
  m_selectedModel = new QStringListModel(parent);
  m_btAdd = new QPushButton("Add ->", parent);
  m_btRemove = new QPushButton("<- Remove", parent);

  m_buttonsLayout = new QVBoxLayout(parent);

  m_allowedListView->setModel(m_allowedModel);
  m_selectedListView->setModel(m_selectedModel);

  // allow multiple selection
  m_allowedListView->setSelectionMode(QAbstractItemView::MultiSelection);
  m_selectedListView->setSelectionMode(QAbstractItemView::MultiSelection);

  m_buttonsLayout->addWidget(m_btAdd);
  m_buttonsLayout->addWidget(m_btRemove);

  m_layout->addWidget(m_allowedListView);
  m_layout->addLayout(m_buttonsLayout);;
  m_layout->addWidget(m_selectedListView);

  connect(m_btAdd, SIGNAL(clicked()), this, SLOT(btAddClicked()));
  connect(m_btRemove, SIGNAL(clicked()), this, SLOT(btRemoveClicked()));

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
      anyToStringList<bool>(opt->value<bool>(), valList);
    }
    else if(type.compare(XmlTag<CF::Real>::type()) == 0)     // Real option
    {
      vectToStringList<CF::Real>(vect, restrList);
      anyToStringList<CF::Real>(opt->value<CF::Real>(), valList);
    }
    else if(type.compare(XmlTag<int>::type()) == 0)          // int option
    {
      vectToStringList<int>(vect, restrList);
      anyToStringList<int>(opt->value<int>(), valList);
    }
    else if(type.compare(XmlTag<CF::Uint>::type()) == 0)     // Uint option
    {
      vectToStringList<CF::Uint>(vect, restrList);
      anyToStringList<CF::Uint>(opt->value<CF::Uint>(), valList);
    }
    else if(type.compare(XmlTag<std::string>::type()) == 0)  // string option
    {
      vectToStringList<std::string>(vect, restrList);
      anyToStringList<std::string>(opt->value<std::string>(), valList);
    }
    else if(type.compare(XmlTag<URI>::type()) == 0)          // URI option
    {
      vectToStringList<URI>(vect, restrList);
      anyToStringList<URI>(opt->value<URI>(), valList);
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
  delete m_allowedListView;
  delete m_selectedListView;
  delete m_allowedModel;
  delete m_selectedModel;
  delete m_buttonsLayout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::setRestrictedList(const QStringList & list)
{
  m_allowedModel->setStringList(list);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalArrayRestrictedList::setValue(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::StringList)
  {
    m_selectedModel->setStringList(value.toStringList());
    m_originalValue = value;
  }

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalArrayRestrictedList::value() const
{
  return m_selectedModel->stringList();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::btAddClicked()
{
  QItemSelectionModel * selectionModel = m_allowedListView->selectionModel();
  QModelIndexList list = selectionModel->selectedIndexes();

  if( !list.empty() )
  {
    QStringList stringList = m_selectedModel->stringList();
    const QAbstractItemModel * model = selectionModel->model();
    QModelIndexList::const_iterator it = list.begin();

    for( ; it != list.end() ; it++)
      stringList << model->data(*it).toString();

    m_selectedModel->setStringList(stringList);

    m_allowedListView->clearSelection();

    emit valueChanged();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::btRemoveClicked()
{
  QItemSelectionModel * selectionModel = m_selectedListView->selectionModel();
  QModelIndexList list = selectionModel->selectedIndexes();

  if( !list.empty() )
  {
    QStringList stringList = m_selectedModel->stringList();
    //const QAbstractItemModel * model = selectionModel->model();
    QModelIndexList::const_iterator it = list.end();

    for( ; it >= list.begin() ; it--)
    {
      if(it != list.end())
        stringList.removeAt( it->row() );
    }

    m_selectedModel->setStringList(stringList);

    m_allowedListView->clearSelection();

    emit valueChanged();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
