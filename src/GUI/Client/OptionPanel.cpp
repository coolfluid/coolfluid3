#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>

#include "Common/CF.hpp"

#include "GUI/Client/CommitDetails.hpp"
#include "GUI/Client/CommitDetailsDialog.hpp"
#include "GUI/Client/ConfirmCommitDialog.hpp"
#include "GUI/Client/GraphicalOption.hpp"
#include "GUI/Client/UnknownTypeException.hpp"
#include "GUI/Client/OptionType.hpp"
#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"

#include "GUI/Client/OptionPanel.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;


OptionPanel::OptionPanel(QWidget * parent)
  : QWidget(parent),
    m_modelReset(false)
{
  NTree::Ptr tree = ClientRoot::getTree();

  // create the components
  m_scrollBasicOptions = new QScrollArea(this);
  m_scrollAdvancedOptions = new QScrollArea(this);
  m_gbBasicOptions = new QGroupBox(m_scrollBasicOptions);
  m_gbAdvancedOptions = new QGroupBox(m_scrollAdvancedOptions);
  m_btApply = new QPushButton("Apply");
  m_btSeeChanges = new QPushButton("See changes");
  m_btForget = new QPushButton("Forget");
  m_splitter = new QSplitter(this);

  m_mainLayout = new QGridLayout(this);
  m_topLayout = new QGridLayout();
  m_basicOptionsLayout = new QFormLayout(m_gbBasicOptions);
  m_advancedOptionsLayout = new QFormLayout(m_gbAdvancedOptions);
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

  m_scrollBasicOptions->setVisible(false);
  m_scrollAdvancedOptions->setVisible(false);
  this->setButtonsVisible(false);

  // on MacOSX, GUI guidelines define the default behaviour for graphical
  // components size in a FormLayout as size hint (they don't take all
  // the space). Since it's extremely ugly (in the client case, they take
  // about 10% of the availbale space), we override the rule by changing
  // the field growth policy.
  m_basicOptionsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  m_advancedOptionsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  connect(m_btApply, SIGNAL(clicked()), this, SLOT(btApplyClicked()));
  connect(m_btSeeChanges, SIGNAL(clicked()), this, SLOT(btSeeChangesClicked()));
  connect(m_btForget, SIGNAL(clicked()), this, SLOT(btForgetClicked()));

  connect(tree.get(), SIGNAL(currentIndexChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(currentIndexChanged(const QModelIndex &, const QModelIndex &)));

  connect(tree.get(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));

  connect(tree.get(), SIGNAL(advancedModeChanged(bool)),
          this, SLOT(advancedModeChanged(bool)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OptionPanel::~OptionPanel()
{
  this->clearList(m_basicOptions);
  this->clearList(m_advancedOptions);

  delete m_btApply;
  delete m_gbBasicOptions;
  delete m_gbAdvancedOptions;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getOptions(QMap<QString, QString> & options) const
{
  options.clear();

  this->buildOptions(m_basicOptions, options);
  this->buildOptions(m_advancedOptions, options);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::buildOptions(const QList<GraphicalOption *> & graphOptions,
                               QMap<QString, QString> & options) const
{
  QList<GraphicalOption *>::const_iterator it = graphOptions.begin();

  for( ; it != graphOptions.end() ; it++)
  {
    GraphicalOption * gOption = *it;

    if(gOption->isModified())
      options[ gOption->getName() ] = gOption->getValueString();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::clearList(QList<GraphicalOption *> & list)
{
  QList<GraphicalOption *>::iterator it = list.begin();

  while(it != list.end())
  {
    delete *it;
    it++;
  }

  list.clear();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::setOptions(const QList<NodeOption> & list)
{
  QList<NodeOption>::const_iterator it = list.begin();
  const NTree::Ptr & tree = ClientRoot::getTree();

  // delete old widgets
  this->clearList(m_basicOptions);
  this->clearList(m_advancedOptions);

 // set the new widgets
  if(!list.isEmpty())
  {
    // get a UNIX-like path for the node
    //   QDomNode parentNode = m_options.at(0).parentNode();
    QString parentPath = tree->getNodePath(tree->getCurrentIndex());

    m_gbBasicOptions->setTitle(QString("Basic options of %1").arg(parentPath));
    m_gbAdvancedOptions->setTitle(QString("Advanced options of %1").arg(parentPath));
    m_currentPath = parentPath;

    // To avoid confusion, basic options panel is always showed if there is at
    // least one option for the selected object, even if all options are advanced.
    // Doing so, we ensure that the advanced options panel is *always* the
    // middle one (if visible) and never the top one.
    m_scrollBasicOptions->setVisible(true);

    this->setButtonsVisible(true);
  }
  else
  {
    m_scrollBasicOptions->setVisible(false);
    m_scrollAdvancedOptions->setVisible(false);
    this->setButtonsVisible(false);
  }

  while(it != list.end())
  {
    GraphicalOption * graphicalOption;

    NodeOption param = *it;
    OptionType::Type type = param.m_paramType;
    bool advanced = param.m_paramAdv;

    // create the graphical component
    try
    {
      graphicalOption = new GraphicalOption(type, this);
      graphicalOption->setName(param.m_paramName);
      graphicalOption->setValue(param.m_paramValue.trimmed());

      graphicalOption->setToolTip(param.m_paramDescr);

      // if this is a basic option...
      if(!advanced)
      {
        m_basicOptions.append(graphicalOption);
        graphicalOption->addToLayout(m_basicOptionsLayout);
      }
      else     // ...or an advanced option
      {
        m_advancedOptions.append(graphicalOption);
        graphicalOption->addToLayout(m_advancedOptionsLayout);
      }

      connect(graphicalOption, SIGNAL(valueChanged()), this, SLOT(valueChanged()));

    }
    catch(UnknownTypeException ute)
    {
      ClientRoot::getLog()->addException(ute.what());
    }

    it++;

  } // end of "while()"

  // change row stretch and panel visibilities
  this->advancedModeChanged(tree->isAdvancedMode());
  this->setButtonsEnabled(false);

  // set options to enabled or disabled (depending on their mode)
//  this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//  this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionPanel::isModified() const
{
  return this->isModified(m_basicOptions) || this->isModified(m_advancedOptions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getModifiedOptions(CommitDetails & commitDetails) const
{
  commitDetails.clear();
  commitDetails.setNodePath(m_currentPath);

  // basic m_options
  this->getModifiedOptions(m_basicOptions, commitDetails);

  // advanced m_options
  this->getModifiedOptions(m_advancedOptions, commitDetails);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString OptionPanel::getCurrentPath() const
{
  return m_currentPath;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 // PRIVATE METHOD

void OptionPanel::getModifiedOptions(const QList<GraphicalOption *> & graphicalOptions,
                                     CommitDetails & commitDetails) const
{
  QList<GraphicalOption *>::const_iterator it = graphicalOptions.begin();

  while(it != graphicalOptions.end())
  {

    GraphicalOption * graphicalOption = *it;

    if(graphicalOption->isModified())
    {
      QString oldValue = graphicalOption->getOrginalValueString();
      QString newValue = graphicalOption->getValueString();

      commitDetails.setOption(graphicalOption->getName(), oldValue, newValue);
    }

    it++;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionPanel::isModified(const QList<GraphicalOption *> & graphicalOptions) const
{
  bool modified = false;

  QList<GraphicalOption *>::const_iterator it = graphicalOptions.begin();

  while(it != graphicalOptions.end() && !modified)
  {
    modified = (*it)->isModified();
    it++;
  }

  return modified;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::setButtonsVisible(bool visible)
{
  m_btApply->setVisible(visible);
  m_btSeeChanges->setVisible(visible);
  m_btForget->setVisible(visible);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::setButtonsEnabled(bool enabled)
{
  m_btApply->setEnabled(enabled);
  m_btSeeChanges->setEnabled(enabled);
  m_btForget->setEnabled(enabled);
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void OptionPanel::btApplyClicked()
{
  QMap<QString, QString> options;
  this->getOptions(options);

  // if there is at least one option that has been modified
  if(!options.isEmpty())
  {
    try
    {
      QModelIndex currentIndex = ClientRoot::getTree()->getCurrentIndex();
      QList<GraphicalOption*>::iterator itBasic = m_basicOptions.begin();
      QList<GraphicalOption*>::iterator itAdv = m_advancedOptions.begin();

      ClientRoot::getTree()->modifyOptions(currentIndex, options);

      for( ; itBasic < m_basicOptions.end() ; itBasic++)
        (*itBasic)->commit();

      for( ; itAdv < m_advancedOptions.end() ; itAdv++)
        (*itAdv)->commit();
    }
    catch (ValueNotFound & vnf)
    {
      ClientRoot::getLog()->addException(vnf.msg().c_str());
    }
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  if(!ClientRoot::getTree()->haveSameData(newIndex, oldIndex))
  {
    QList<NodeOption> params;
    ClientRoot::getTree()->getNodeOptions(newIndex, params);
    this->setOptions(params);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::advancedModeChanged(bool advanced)
{
  m_scrollAdvancedOptions->setVisible(advanced && !m_advancedOptions.empty());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::dataChanged(const QModelIndex & first, const QModelIndex & last)
{
  QModelIndex currIndex = ClientRoot::getTree()->getCurrentIndex();

  if(first == last && first.row() == currIndex.row() && first.parent() == currIndex.parent())
    this->currentIndexChanged(first, QModelIndex());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::btSeeChangesClicked()
{
  CommitDetails details;
  CommitDetailsDialog dialog;

  this->getModifiedOptions(details);
  dialog.show(details);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::btForgetClicked()
{
  this->currentIndexChanged(ClientRoot::getTree()->getCurrentIndex(), QModelIndex());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::valueChanged()
{
  this->setButtonsEnabled(this->isModified());
}
