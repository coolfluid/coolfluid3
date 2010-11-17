// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QComboBox>
#include <QVBoxLayout>

#include "Common/URI.hpp"

#include "GUI/Client/UI/GraphicalRestrictedList.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientUI;

GraphicalRestrictedList::GraphicalRestrictedList(Option::ConstPtr opt, QWidget * parent)
  : GraphicalValue(parent)
{
  const std::vector<boost::any> & vect = opt->restricted_list();

  QStringList list;

  m_comboChoices = new QComboBox(this);

  if(opt.get() != nullptr && opt->has_restricted_list())
  {
    std::string type = opt->type();

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

    m_comboChoices->addItems(list);
  }

  setValue(m_comboChoices->currentText());

  m_layout->addWidget(m_comboChoices);

  connect(m_comboChoices, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

GraphicalRestrictedList::~GraphicalRestrictedList()
{
  delete m_comboChoices;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalRestrictedList::setRestrictedList(const QStringList & list)
{
  m_comboChoices->clear();
  m_comboChoices->addItems(list);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GraphicalRestrictedList::setValue(const QVariant & value)
{
  bool valid = false;

  if(value.type() == QVariant::String)
  {
    int index = m_comboChoices->findText(value.toString());

    if(index > -1)
    {
      m_originalValue = value;
      m_comboChoices->setCurrentIndex(index);
      valid = true;
    }
  }

  return valid;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant GraphicalRestrictedList::value() const
{
  return m_comboChoices->currentText();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GraphicalRestrictedList::currentIndexChanged(int)
{
  emit valueChanged();
}
