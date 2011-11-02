// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QVBoxLayout>

#include "common/StringConversion.hpp"
#include "common/URI.hpp"

#include "UI/Graphics/GraphicalRestrictedList.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

/////////////////////////////////////////////////////////////////////////////

GraphicalRestrictedList::GraphicalRestrictedList(Option::ConstPtr opt, QWidget * parent)
  : GraphicalValue(parent)
{
  m_comboChoices = new QComboBox(this);

  if(opt.get() != nullptr && opt->has_restricted_list())
  {
    QStringList list;
    std::string type = opt->type();
    const std::vector<boost::any> & vect = opt->restricted_list();

    if(type == "bool")              // bool option
      vect_to_stringlist<bool>(vect, list);
    else if(type == "real")         // Real option
      vect_to_stringlist<Real>(vect, list);
    else if(type == "integer")          // int option
      vect_to_stringlist<int>(vect, list);
    else if(type == "unsigned")         // Uint option
      vect_to_stringlist<Uint>(vect, list);
    else if(type == "string")  // string option
      vect_to_stringlist<std::string>(vect, list);
    else if(type == "uri")          // URI option
      vect_to_stringlist<URI>(vect, list);
    else
      throw CastingFailed(FromHere(), type + ": Unknown type");

    set_restricted_list(list);
  }

  set_value(m_comboChoices->currentText());

  m_layout->addWidget(m_comboChoices);

  connect(m_comboChoices, SIGNAL(currentIndexChanged(int)), this, SLOT(current_index_changed(int)));
}

/////////////////////////////////////////////////////////////////////////////

GraphicalRestrictedList::~GraphicalRestrictedList()
{
  delete m_comboChoices;
}

/////////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedList::set_restricted_list(const QStringList & list)
{
  QStringList mList(list);
  mList.removeAll("");
  mList.removeDuplicates();

  m_comboChoices->clear();
  m_comboChoices->addItems(mList);

  if(!mList.isEmpty())
    set_value(mList.front());
}

/////////////////////////////////////////////////////////////////////////////

bool GraphicalRestrictedList::set_value(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::String)
  {
    int index = m_comboChoices->findText(value.toString());

    if(index > -1)
    {
      m_original_value = value;
      m_comboChoices->setCurrentIndex(index); // emits current_index_changed() signal
      valid = true;
    }
  }

  return valid;
}

/////////////////////////////////////////////////////////////////////////////

QVariant GraphicalRestrictedList::value() const
{
  return m_comboChoices->currentText();
}

/////////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedList::current_index_changed(int)
{
  emit value_changed();
}

/////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
void GraphicalRestrictedList::vect_to_stringlist(const std::vector<boost::any> & vect,
                      QStringList & list) const
{
  std::vector<boost::any>::const_iterator it = vect.begin();

  for( ; it != vect.end() ; it++)
    list << to_str( boost::any_cast<TYPE>(*it) ).c_str();
}

/////////////////////////////////////////////////////////////////////////////

#define TEMPLATE_EXPLICIT_INSTANTIATON(T) \
Common_TEMPLATE template void GraphicalRestrictedList::vect_to_stringlist<T>(\
    const std::vector<boost::any>&,QStringList&) const


TEMPLATE_EXPLICIT_INSTANTIATON( bool );
TEMPLATE_EXPLICIT_INSTANTIATON( int );
TEMPLATE_EXPLICIT_INSTANTIATON( cf3::Uint );
TEMPLATE_EXPLICIT_INSTANTIATON( cf3::Real );
TEMPLATE_EXPLICIT_INSTANTIATON( std::string );
TEMPLATE_EXPLICIT_INSTANTIATON( URI );

#undef TEMPLATE_EXPLICIT_INSTANTIATON

/////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
