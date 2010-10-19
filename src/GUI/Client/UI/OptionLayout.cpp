// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDebug>

#include "Common/CF.hpp"

#include "GUI/Client/Core/CommitDetails.hpp"
#include "GUI/Client/UI/CommitDetailsDialog.hpp"
#include "GUI/Client/UI/ConfirmCommitDialog.hpp"
#include "GUI/Client/UI/GraphicalValue.hpp"
#include "GUI/Client/Core/UnknownTypeException.hpp"
#include "GUI/Client/Core/OptionType.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/UI/OptionLayout.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;


OptionLayout::OptionLayout(QWidget * parent)
  : QFormLayout(parent)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OptionLayout::~OptionLayout()
{
  this->clearOptions();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionLayout::getOptions(QMap<QString, QString> & options) const
{
  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() ; it++)
  {
    GraphicalValue * value = *it;
    QLabel * label = static_cast<QLabel*>(labelForField(*it));

    if(value->isModified())
      options[ label->text() ] = value->getValueString();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionLayout::clearOptions()
{
  QList<GraphicalValue *>::iterator it = m_options.begin();

  while(it != m_options.end())
  {
    delete labelForField(*it); // delete the associated label
    delete *it;
    it++;
  }

  m_options.clear();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionLayout::isModified() const
{
  bool modified = false;

  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() && !modified ; it++)
    modified = (*it)->isModified();

  return modified;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionLayout::getModifiedOptions(CommitDetails & commitDetails) const
{
  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  while(it != m_options.end())
  {
    GraphicalValue * value = *it;

    if(value->isModified())
    {
      QString oldValue = value->getOriginalValueString();
      QString newValue = value->getValueString();
      QLabel * label = static_cast<QLabel*>(labelForField(*it));

      commitDetails.setOption(label->text(), oldValue, newValue);
    }

    it++;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionLayout::addOption(CF::Common::Option::ConstPtr option)
{
  GraphicalValue * value = GraphicalValue::create(option, static_cast<QWidget*>(this->parent()));

  m_options.append(value);

  value->setToolTip(option->description().c_str());

  addRow(option->name().c_str(), value);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionLayout::hasOptions() const
{
  return !m_options.isEmpty();
}

/****************************************************************************

 PRIVATE METHOD

 ****************************************************************************/

bool OptionLayout::isModified(const QList<GraphicalValue *> & graphicalOptions) const
{
  bool modified = false;

  QList<GraphicalValue *>::const_iterator it = graphicalOptions.begin();

  while(it != graphicalOptions.end() && !modified)
  {
    modified = (*it)->isModified();
    it++;
  }

  return modified;
}
