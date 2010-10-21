// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>

#include "Common/CF.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/CommitDetails.hpp"
#include "GUI/Client/Core/CNode.hpp"
#include "GUI/Client/Core/UnknownTypeException.hpp"

#include "GUI/Client/UI/CommitDetailsDialog.hpp"
#include "GUI/Client/UI/ConfirmCommitDialog.hpp"
#include "GUI/Client/UI/GraphicalValue.hpp"
#include "GUI/Client/UI/OptionLayout.hpp"

#include "GUI/Client/UI/CentralPanel.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;


CentralPanel::CentralPanel(QWidget * parent)
  : QWidget(parent),
    m_modelReset(false)
{
  NTree::Ptr tree = ClientRoot::tree();

  // create the components
  m_scrollBasicOptions = new QScrollArea();
  m_scrollAdvancedOptions = new QScrollArea();
  m_gbBasicOptions = new QGroupBox(m_scrollBasicOptions);
  m_gbAdvancedOptions = new QGroupBox(m_scrollAdvancedOptions);
  m_btApply = new QPushButton("Apply");
  m_btSeeChanges = new QPushButton("See changes");
  m_btForget = new QPushButton("Forget");
  m_splitter = new QSplitter(this);

  m_basicOptionLayout = new OptionLayout(m_gbBasicOptions);
  m_advancedOptionLayout = new OptionLayout(m_gbAdvancedOptions);

  m_mainLayout = new QGridLayout(this);
  m_topLayout = new QGridLayout();
  m_buttonsLayout = new QGridLayout();

  m_mainLayout->setContentsMargins(0, 11, 0, 0);

  m_splitter->setOrientation(Qt::Vertical);

  m_scrollBasicOptions->setWidgetResizable(true);
  m_scrollBasicOptions->setWidget(m_gbBasicOptions);

  m_scrollAdvancedOptions->setWidgetResizable(true);
  m_scrollAdvancedOptions->setWidget(m_gbAdvancedOptions);

  m_btSeeChanges->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_btForget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_btApply->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  // add the components to the layout
  m_splitter->addWidget(m_scrollBasicOptions);
  m_splitter->addWidget(m_scrollAdvancedOptions);

  m_topLayout->addWidget(new QWidget(this), 0, 0);
  m_topLayout->addWidget(m_btSeeChanges, 0, 1);

  m_buttonsLayout->addWidget(m_btForget, 0, 0);
  m_buttonsLayout->addWidget(new QWidget(), 0, 1);
  m_buttonsLayout->addWidget(m_btApply, 0, 2);

  m_mainLayout->addLayout(m_topLayout, 0, 0);
  m_mainLayout->addWidget(m_splitter, 1, 0);
  m_mainLayout->addLayout(m_buttonsLayout, 2, 0);

  advancedModeChanged(tree->isAdvancedMode());

  this->setButtonsVisible(false);

  connect(m_btApply, SIGNAL(clicked()), this, SLOT(btApplyClicked()));
  connect(m_btSeeChanges, SIGNAL(clicked()), this, SLOT(btSeeChangesClicked()));
  connect(m_btForget, SIGNAL(clicked()), this, SLOT(btForgetClicked()));

  connect(tree.get(), SIGNAL(currentIndexChanged(QModelIndex, QModelIndex)),
          this, SLOT(currentIndexChanged(QModelIndex, QModelIndex)));

  connect(tree.get(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
          this, SLOT(dataChanged(QModelIndex, QModelIndex)));

  connect(tree.get(), SIGNAL(advancedModeChanged(bool)),
          this, SLOT(advancedModeChanged(bool)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CentralPanel::~CentralPanel()
{
  // buttons
  delete m_btApply;
  delete m_btForget;
  delete m_btSeeChanges;

  // layouts
  delete m_buttonsLayout;
  delete m_topLayout;
  delete m_basicOptionLayout;
  delete m_advancedOptionLayout;

  // group boxes
  delete m_gbBasicOptions;
  delete m_gbAdvancedOptions;

  // scroll areas
  delete m_scrollAdvancedOptions;
  delete m_scrollBasicOptions;

  // splitter
  delete m_splitter;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::setOptions(const QList<Option::ConstPtr> & list)
{
  QList<Option::ConstPtr>::const_iterator it = list.begin();
  const NTree::Ptr & tree = ClientRoot::tree();

  // delete old options
  m_basicOptionLayout->clearOptions();
  m_advancedOptionLayout->clearOptions();

  // if there is at least one option, we set the group boxes title
  if(!list.isEmpty())
  {
    // get a UNIX-like path for the node
    QString parentPath = tree->getNodePath(tree->getCurrentIndex());

    m_gbBasicOptions->setTitle(QString("Basic options of %1").arg(parentPath));
    m_gbAdvancedOptions->setTitle(QString("Advanced options of %1").arg(parentPath));
    m_currentPath = parentPath;
  }

  while(it != list.end())
  {
    // create the option
    try
    {
      Option::ConstPtr option = *it;
      bool basic = option->has_tag("basic");

      if (basic)
        m_basicOptionLayout->addOption(option);
      else
        m_advancedOptionLayout->addOption(option);

    }
    catch(UnknownTypeException ute)
    {
      ClientRoot::log()->addException(ute.what());
    }

    it++;

  } // end of "while()"

  // change row stretch and panel visibilities
  this->advancedModeChanged(tree->isAdvancedMode());
  this->setButtonsVisible(!list.isEmpty());
  this->setButtonsEnabled(false);

  // set options to enabled or disabled (depending on their mode)
//  this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//  this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CentralPanel::isModified() const
{
  return m_basicOptionLayout->isModified() || m_advancedOptionLayout->isModified();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::getModifiedOptions(CommitDetails & commitDetails) const
{
  commitDetails.clear();
  commitDetails.setNodePath(m_currentPath);

  m_basicOptionLayout->getModifiedOptions(commitDetails);
  m_advancedOptionLayout->getModifiedOptions(commitDetails);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CentralPanel::getCurrentPath() const
{
  return m_currentPath;
}

/****************************************************************************

 PRIVATE METHODS

 ****************************************************************************/


void CentralPanel::setButtonsVisible(bool visible)
{
  m_btApply->setVisible(visible);
  m_btSeeChanges->setVisible(visible);
  m_btForget->setVisible(visible);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::setButtonsEnabled(bool enabled)
{
  m_btApply->setEnabled(enabled);
  m_btSeeChanges->setEnabled(enabled);
  m_btForget->setEnabled(enabled);
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void CentralPanel::btApplyClicked()
{
  QMap<QString, QString> options;


  //this->getOptions(options);

  // if there is at least one option that has been modified
  if(!options.isEmpty())
  {
    try
    {
      QModelIndex currentIndex = ClientRoot::tree()->getCurrentIndex();
//      QList<GraphicalValue*>::iterator itBasic = m_basicOptions.begin();
//      QList<GraphicalValue*>::iterator itAdv = m_advancedOptions.begin();

      ClientRoot::tree()->modifyOptions(currentIndex, options);

//      for( ; itBasic < m_basicOptions.end() ; itBasic++)
//        (*itBasic)->commit();

//      for( ; itAdv < m_advancedOptions.end() ; itAdv++)
//        (*itAdv)->commit();
    }
    catch (ValueNotFound & vnf)
    {
      ClientRoot::log()->addException(vnf.msg().c_str());
    }
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  QList<Option::ConstPtr> options;
  ClientRoot::tree()->getNodeOptions(newIndex, options);
  this->setOptions(options);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::advancedModeChanged(bool advanced)
{
  m_scrollAdvancedOptions->setVisible(advanced && !m_advancedOptionLayout->hasOptions());

  // To avoid confusion, basic option panel is always showed if there is at
  // least one option for the selected object, even if all options are advanced.
  // Doing so, we ensure that the advanced options panel is *never* the
  // top one (if visible).
  m_scrollBasicOptions->setVisible(m_basicOptionLayout->hasOptions() || m_scrollAdvancedOptions->isVisible());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::dataChanged(const QModelIndex & first, const QModelIndex & last)
{
  QModelIndex currIndex = ClientRoot::tree()->getCurrentIndex();

  if(first == last && first.row() == currIndex.row() && first.parent() == currIndex.parent())
    this->currentIndexChanged(first, QModelIndex());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::btSeeChangesClicked()
{
  CommitDetails details;
  CommitDetailsDialog dialog;

  this->getModifiedOptions(details);
  dialog.show(details);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::btForgetClicked()
{
  this->currentIndexChanged(ClientRoot::tree()->getCurrentIndex(), QModelIndex());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CentralPanel::valueChanged()
{
  this->setButtonsEnabled(this->isModified());
}
