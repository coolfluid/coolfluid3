// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include <QListView>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>

#include "common/OptionArray.hpp"
#include "common/StringConversion.hpp"
#include "common/URI.hpp"

#include "ui/core/TreeThread.hpp"

#include "ui/graphics/GraphicalArrayRestrictedList.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

/////////////////////////////////////////////////////////////////////////

GraphicalArrayRestrictedList::GraphicalArrayRestrictedList(boost::shared_ptr< Option > opt,
                                                           QWidget * parent)
{
  QStringList restrList;
  QStringList valList;

  m_group_box = new QGroupBox(parent);
  m_allowed_list_view = new QListView(m_group_box);
  m_allowed_model = new QStringListModel(m_group_box);
  m_selected_list_view = new QListView(m_group_box);
  m_selected_model = new QStringListModel(m_group_box);
  m_bt_add = new QPushButton("Add ->", m_group_box);
  m_bt_remove = new QPushButton("<- Remove", m_group_box);

  m_buttons_layout = new QVBoxLayout();
  m_box_layout = new QGridLayout(m_group_box);

  m_allowed_list_view->setModel(m_allowed_model);
  m_selected_list_view->setModel(m_selected_model);

  // allow multiple selection
  m_allowed_list_view->setSelectionMode(QAbstractItemView::MultiSelection);
  m_selected_list_view->setSelectionMode(QAbstractItemView::MultiSelection);

  m_buttons_layout->addWidget(m_bt_add);
  m_buttons_layout->addWidget(m_bt_remove);

  m_box_layout->addWidget(m_allowed_list_view, 0, 0);
  m_box_layout->addLayout(m_buttons_layout, 0, 1);
  m_box_layout->addWidget(m_selected_list_view, 0, 2);


  m_layout->addWidget(m_group_box);

  connect(m_bt_add, SIGNAL(clicked()), this, SLOT(bt_add_clicked()));
  connect(m_bt_remove, SIGNAL(clicked()), this, SLOT(bt_remove_clicked()));

 if(opt.get() != nullptr && boost::starts_with(opt->type(), "array")  &&
    opt->has_restricted_list())
 {
   const std::vector<boost::any> & vect = opt->restricted_list();

   const std::string type = opt->element_type();

   if(type == common::class_name<bool>())              // bool option
   {
     vect_to_stringlist<bool>(vect, restrList);
     any_to_stringlist<bool>(opt->value(), valList);
   }
   else if(type == common::class_name<Real>())     // Real option
   {
     vect_to_stringlist<cf3::Real>(vect, restrList);
     any_to_stringlist<cf3::Real>(opt->value(), valList);
   }
   else if(type == common::class_name<int>())          // int option
   {
     vect_to_stringlist<int>(vect, restrList);
     any_to_stringlist<int>(opt->value(), valList);
   }
   else if(type == common::class_name<Uint>())     // Uint option
   {
     vect_to_stringlist<cf3::Uint>(vect, restrList);
     any_to_stringlist<cf3::Uint>(opt->value(), valList);
   }
   else if(type == common::class_name<std::string>())  // string option
   {
     vect_to_stringlist<std::string>(vect, restrList);
     any_to_stringlist<std::string>(opt->value(), valList);
   }
   else if(type == common::class_name<URI>())          // URI option
   {
     vect_to_stringlist<URI>(vect, restrList);
     any_to_stringlist<URI>(opt->value(), valList);
   }
   else
     throw CastingFailed(FromHere(), type + ": Unknown type");

   set_restricted_list(restrList);
   set_value(valList);
 }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalArrayRestrictedList::~GraphicalArrayRestrictedList()
{
  delete m_allowed_list_view;
  delete m_selected_list_view;
  delete m_allowed_model;
  delete m_selected_model;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::set_restricted_list(const QStringList & list)
{
  m_allowed_model->setStringList(list);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalArrayRestrictedList::set_value(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::StringList)
  {
    m_selected_model->setStringList(value.toStringList());
    m_original_value = value;
  }

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalArrayRestrictedList::value() const
{
  return m_selected_model->stringList();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::bt_add_clicked()
{
  QItemSelectionModel * selectionModel = m_allowed_list_view->selectionModel();
  QModelIndexList list = selectionModel->selectedIndexes();

  if( !list.empty() )
  {
    QStringList stringList = m_selected_model->stringList();
    const QAbstractItemModel * model = selectionModel->model();
    QModelIndexList::const_iterator it = list.begin();

    for( ; it != list.end() ; it++)
      stringList << model->data(*it).toString();

    m_selected_model->setStringList(stringList);

    m_allowed_list_view->clearSelection();

    emit value_changed();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalArrayRestrictedList::bt_remove_clicked()
{
  QItemSelectionModel * selectionModel = m_selected_list_view->selectionModel();
  QModelIndexList list = selectionModel->selectedIndexes();

  if( !list.empty() )
  {
    QStringList stringList = m_selected_model->stringList();
    //const QAbstractItemModel * model = selectionModel->model();
    QModelIndexList::const_iterator it = list.end();

    for( ; it >= list.begin() ; it--)
    {
      if(it != list.end())
        stringList.removeAt( it->row() );
    }

    m_selected_model->setStringList(stringList);

    m_allowed_list_view->clearSelection();

    emit value_changed();
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename TYPE>
void GraphicalArrayRestrictedList::vect_to_stringlist(const std::vector<boost::any> & vect,
                                                    QStringList & list) const
{
  std::vector<boost::any>::const_iterator it = vect.begin();

  try
  {
    for( ; it != vect.end() ; it++)
      list << to_str( boost::any_cast<TYPE>(*it) ).c_str();
  }
  catch(boost::bad_any_cast & bac)
  {
    std::string realType = demangle(it->type().name());
    const std::string typeToCast = common::class_name<TYPE>();

    throw cf3::common::CastingFailed(FromHere(), "Unable to cast [" + realType
                                    + "] to [" + typeToCast +"]");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename TYPE>
void GraphicalArrayRestrictedList::any_to_stringlist(const boost::any & value,
                                                   QStringList & list) const
{
  try
  {
    std::vector<TYPE> vect = boost::any_cast<std::vector<TYPE> >(value);
    typename std::vector<TYPE>::const_iterator it = vect.begin();

    for( ; it != vect.end() ; it++)
      list << to_str(*it).c_str();
  }
  catch(boost::bad_any_cast & bac)
  {
    std::string realType = cf3::common::demangle(value.type().name());
    const std::string typeToCast = common::class_name<TYPE>();

    throw cf3::common::CastingFailed(FromHere(), "Unable to cast [" + realType
                                    + "] to [" + typeToCast +"]");
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define TEMPLATE_EXPLICIT_INSTANTIATON(T) \
Common_TEMPLATE template void GraphicalArrayRestrictedList::vect_to_stringlist<T>(\
                                const std::vector<boost::any>&, QStringList&) const;\
Common_TEMPLATE template void GraphicalArrayRestrictedList::any_to_stringlist<T>(\
                                const boost::any&, QStringList&) const

TEMPLATE_EXPLICIT_INSTANTIATON( bool );
TEMPLATE_EXPLICIT_INSTANTIATON( int );
TEMPLATE_EXPLICIT_INSTANTIATON( cf3::Uint );
TEMPLATE_EXPLICIT_INSTANTIATON( cf3::Real );
TEMPLATE_EXPLICIT_INSTANTIATON( std::string );
TEMPLATE_EXPLICIT_INSTANTIATON( URI );

#undef TEMPLATE_EXPLICIT_INSTANTIATON


//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

