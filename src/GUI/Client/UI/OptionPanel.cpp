// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFormLayout>

#include "Common/CF.hpp"

#include "GUI/Client/Core/CommitDetails.hpp"
#include "GUI/Client/UI/CommitDetailsDialog.hpp"
#include "GUI/Client/UI/ConfirmCommitDialog.hpp"
#include "GUI/Client/UI/GraphicalValue.hpp"
#include "GUI/Client/Core/UnknownTypeException.hpp"
#include "GUI/Client/Core/OptionType.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CNode.hpp"

#include "GUI/Client/UI/OptionPanel.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;


OptionPanel::OptionPanel(QWidget * parent)
  : QWidget(parent)
{
  NTree::Ptr tree = ClientRoot::tree();

  // create the components
  m_mainLayout = new QFormLayout(this);

//  m_mainLayout->setContentsMargins(0, 11, 0, 0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OptionPanel::~OptionPanel()
{
  this->clearOptions();
  delete m_mainLayout;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getOptions(QMap<QString, QString> & options) const
{
  options.clear();

  //  QList<GraphicalValue *>::const_iterator it = graphOptions.begin();

  //  for( ; it != graphOptions.end() ; it++)
  //  {
  //    GraphicalValue * gOption = *it;

  //    if(gOption->isModified())
  //      options[ gOption->getName() ] = gOption->getValueString();
  //  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::clearOptions()
{
  QList<GraphicalValue *>::iterator it = m_options.begin();

  while(it != m_options.end())
  {
    delete m_mainLayout->labelForField(*it); // delete the associated label
    delete *it;
    it++;
  }

  m_options.clear();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionPanel::isModified() const
{
  return true;
//  return this->isModified(m_options) || this->isModified(m_advancedOptions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getModifiedOptions(CommitDetails & commitDetails) const
{
  commitDetails.clear();

  // basic m_options
//  this->getModifiedOptions(m_options, commitDetails);

  // advanced m_options
//  this->getModifiedOptions(m_advancedOptions, commitDetails);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::addOption(CF::Common::Option::ConstPtr option)
{
  GraphicalValue * value = GraphicalValue::create(option, this);
  
  m_options.append(value);
  
  m_mainLayout->addRow(option->name().c_str(), value);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 // PRIVATE METHOD

void OptionPanel::getModifiedOptions(const QList<GraphicalValue *> & graphicalOptions,
                                     CommitDetails & commitDetails) const
{
  throw NotImplemented(FromHere(), "OptionPanel::getModifiedOptions()");

//  QList<GraphicalValue *>::const_iterator it = graphicalOptions.begin();

//  while(it != graphicalOptions.end())
//  {

//    GraphicalValue * graphicalValue = *it;

//    if(graphicalValue->isModified())
//    {
//      QString oldValue = graphicalValue->getOrginalValueString();
//      QString newValue = graphicalValue->getValueString();

//      commitDetails.setOption(graphicalValue->getName(), oldValue, newValue);
//    }

//    it++;
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionPanel::isModified(const QList<GraphicalValue *> & graphicalOptions) const
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
