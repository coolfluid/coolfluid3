// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QVBoxLayout>

#include "Common/StringConversion.hpp"
#include "Common/URI.hpp"

#include "UI/Graphics/GraphicalRestrictedList.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

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
      vectToStringList<bool>(vect, list);
    else if(type == "real")         // Real option
      vectToStringList<Real>(vect, list);
    else if(type == "integer")          // int option
      vectToStringList<int>(vect, list);
    else if(type == "unsigned")         // Uint option
      vectToStringList<Uint>(vect, list);
    else if(type == "string")  // string option
      vectToStringList<std::string>(vect, list);
    else if(type == "uri")          // URI option
      vectToStringList<URI>(vect, list);
    else
      throw CastingFailed(FromHere(), type + ": Unknown type");

    setRestrictedList(list);
  }

  setValue(m_comboChoices->currentText());

  m_layout->addWidget(m_comboChoices);

  connect(m_comboChoices, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}

/////////////////////////////////////////////////////////////////////////////

GraphicalRestrictedList::~GraphicalRestrictedList()
{
  delete m_comboChoices;
}

/////////////////////////////////////////////////////////////////////////////

void GraphicalRestrictedList::setRestrictedList(const QStringList & list)
{
  QStringList mList(list);
  mList.removeAll("");
  mList.removeDuplicates();

  m_comboChoices->clear();
  m_comboChoices->addItems(mList);

  if(!mList.isEmpty())
    setValue(mList.front());
}

/////////////////////////////////////////////////////////////////////////////

bool GraphicalRestrictedList::setValue(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::String)
  {
    int index = m_comboChoices->findText(value.toString());

    if(index > -1)
    {
      m_originalValue = value;
      m_comboChoices->setCurrentIndex(index); // emits currentIndexChanged() signal
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

void GraphicalRestrictedList::currentIndexChanged(int)
{
  emit valueChanged();
}

/////////////////////////////////////////////////////////////////////////////

template<typename TYPE>
void GraphicalRestrictedList::vectToStringList(const std::vector<boost::any> & vect,
                      QStringList & list) const
{
  std::vector<boost::any>::const_iterator it = vect.begin();

  for( ; it != vect.end() ; it++)
    list << to_str( boost::any_cast<TYPE>(*it) ).c_str();
}

/////////////////////////////////////////////////////////////////////////////

#define TEMPLATE_EXPLICIT_INSTANTIATON(T) \
Common_TEMPLATE template void GraphicalRestrictedList::vectToStringList<T>(\
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
