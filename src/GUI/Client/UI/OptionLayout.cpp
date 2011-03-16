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
#include "GUI/Client/Core/TreeThread.hpp"
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

void OptionLayout::options(QMap<QString, QString> & options, bool all) const
{
  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() ; it++)
  {
    GraphicalValue * value = *it;
    QLabel * label = static_cast<QLabel*>(labelForField(*it));

    if(all || value->isModified())
      options[ label->text() ] = value->valueString();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionLayout::commitOpions()
{
  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() ; it++)
    (*it)->commit();
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

void OptionLayout::modifiedOptions(CommitDetails & commitDetails) const
{
  QList<GraphicalValue *>::const_iterator it = m_options.begin();

  while(it != m_options.end())
  {
    GraphicalValue * value = *it;

    if(value->isModified())
    {
      QString oldValue = value->originalValueString();
      QString newValue = value->valueString();
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
  GraphicalValue * value = GraphicalValue::createFromOption(option);

  m_options.append(value);

  value->setToolTip(option->description().c_str());

  addRow(option->name().c_str(), value);

  // forward the signal
  connect(value, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionLayout::hasOptions() const
{
  return !m_options.isEmpty();
}
