#include <iostream>

#include <QtCore>
#include <QtGui>

#include "GUI/Client/CloseConfirmationInfos.hpp"
#include "GUI/Client/CommitDetails.hpp"
#include "GUI/Client/CommitDetailsDialog.hpp"
#include "GUI/Client/GraphicalOption.hpp"
#include "GUI/Client/UnknownTypeException.hpp"
#include "GUI/Client/OptionType.hpp"
#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"

#include "GUI/Client/OptionPanel.hpp"

using namespace CF::GUI::Client;


OptionPanel::OptionPanel(QWidget * parent) : QWidget(parent)
{
  // create the components
  m_scrollBasicOptions = new QScrollArea(this);
  m_scrollAdvancedOptions = new QScrollArea(this);
  m_gbBasicOptions = new QGroupBox(m_scrollBasicOptions);
  m_gbAdvancedOptions = new QGroupBox(m_scrollAdvancedOptions);
  m_btCommitChanges = new QPushButton("Commit changes");
  m_btCheckChanges = new QPushButton("Check changes");
  m_btResetOptions = new QPushButton("Reset changes");
  m_splitter = new QSplitter(this);

  m_mainLayout = new QGridLayout(this);
  m_basicOptionsLayout = new QFormLayout(m_gbBasicOptions);
  m_advancedOptionsLayout = new QFormLayout(m_gbAdvancedOptions);
  m_buttonsLayout = new QHBoxLayout();

  m_splitter->setOrientation(Qt::Vertical);
  m_scrollBasicOptions->setWidgetResizable(true);
  m_scrollBasicOptions->setWidget(m_gbBasicOptions);

  m_scrollAdvancedOptions->setWidgetResizable(true);
  m_scrollAdvancedOptions->setWidget(m_gbAdvancedOptions);

  // add the components to the m_layout
  m_splitter->addWidget(m_scrollBasicOptions);
  m_splitter->addWidget(m_scrollAdvancedOptions);

  m_mainLayout->addWidget(m_splitter, 0, 0);

  m_buttonsLayout->addWidget(m_btCheckChanges);
  m_buttonsLayout->addWidget(m_btCommitChanges);
  m_buttonsLayout->addWidget(m_btResetOptions);

  m_mainLayout->addLayout(m_buttonsLayout, 1, 0);

  m_readOnly = false;
  m_treeModele = CFNULL;
  m_scrollBasicOptions->setVisible(false);
  this->buttonsSetVisible(false);

  connect(m_btCommitChanges, SIGNAL(clicked()), this, SLOT(commitChanges()));
  connect(m_btCheckChanges, SIGNAL(clicked()), this, SLOT(checkOptions()));
  connect(m_btResetOptions, SIGNAL(clicked()), this, SLOT(resetChanges()));

  connect(ClientRoot::getTree().get(), SIGNAL(currentIndexChanged(const QModelIndex &)),
          this, SLOT(currentIndexChanged(const QModelIndex &)));

  connect(ClientRoot::getTree().get(), SIGNAL(advancedModeChanged(bool)),
          this, SLOT(advancedModeChanged(bool)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OptionPanel::~OptionPanel()
{
  this->clearList(m_basicOptions);
  this->clearList(m_advancedOptions);
  this->clearList(m_newBasicOptions);
  this->clearList(m_newAdvancedOptions);

  delete m_btCommitChanges;
  delete m_gbBasicOptions;
  delete m_gbAdvancedOptions;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::setEnabled(const QDomDocument & optionsNodes,
                             const QList<GraphicalOption *> & options)
{
  QDomNodeList nodes = optionsNodes.childNodes();

  for(int i = 0 ; i < nodes.count() ; i++)
  {
    QDomNode currentNode = nodes.at(i);
    QDomNamedNodeMap attributes = currentNode.attributes();
    bool isDynamic = attributes.namedItem("dynamic").nodeValue() == "true";

    if(m_readOnly && isDynamic)
      options.at(i)->setEnabled(true);
    else if(!m_readOnly)
      options.at(i)->setEnabled(true);
    else
      options.at(i)->setEnabled(false);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::addOption(OptionType::Type optionType, const QString & name,
                            bool basic, bool dynamic)
{
//  QDomElement node = m_newBasicOptionsNodes.createElement(name);
//  GraphicalOption * newOption;
//  QString mode = basic ? "basic" : "advanced";
//  QString dynamicStr = QVariant(dynamic).toString();

//  if(node.isNull() || name.isNull() || name.isEmpty())
//    return;

//  QString typeString = OptionType::Convert::to_str(optionType).c_str();

//  if(typeString.isEmpty())
//    throw UnknownTypeException(FromHere(), "Unknown type");

//  node.setAttribute("tree", "option");
//  node.setAttribute("type", typeString);
//  node.setAttribute("mode", mode);
//  node.setAttribute("dynamic", dynamicStr);

//  newOption = new GraphicalOption(optionType);
//  newOption->setName(name + QString(":"));

//  // if the option is basic...
//  if(basic)
//  {
//    newOption->addToLayout(m_basicOptionsLayout);
//    m_newBasicOptionsNodes.appendChild(node);
//    m_newBasicOptions.append(newOption);
//  }
//  else // ...or advanced
//  {
//    newOption->addToLayout(m_advancedOptionsLayout);
//    m_newAdvancedOptionsNodes.appendChild(node);
//    m_newAdvancedOptions.append(newOption);
//  }

//  this->advancedModeChanged(m_advancedMode);
//  this->buttonsSetVisible(true);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDomDocument OptionPanel::getOptions() const
{
  QDomDocument doc;

//  this->buildOptions(m_basicOptionsNodes, m_basicOptions, doc);
//  this->buildOptions(m_advancedOptionsNodes, m_advancedOptions, doc);

  return doc;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::buildOptions(const QDomDocument & nodes,
                               const QList<GraphicalOption *> & options,
                               QDomDocument & document) const
{
  QDomNodeList childNodes = nodes.childNodes();

  for(int i = 0 ; i < childNodes.count() ; i++)
  {
    GraphicalOption * gOption = options.at(i);

    // if the option has been modified, we can add it to the tree
    if(gOption->isModified())
    {
      QDomText nodeValue = document.createTextNode(gOption->getValueString());
      QDomElement newNode;

      // import the node with its XML attributes
      newNode = document.importNode(childNodes.at(i), false).toElement();

      newNode.appendChild(nodeValue);
      document.appendChild(newNode);
    }
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDomDocument OptionPanel::getNewOptions() const
{
  QDomDocument doc;

//  this->buildOptions(m_newBasicOptionsNodes, m_newBasicOptions, doc);
//  this->buildOptions(m_newAdvancedOptionsNodes, m_newAdvancedOptions, doc);

  return doc;
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

//void OptionPanel::setOptions(const QDomNodeList & options)
//{
//  CTree::Ptr & tree = ClientRoot::getTree();

//  // delete old widgets
//  this->clearList(m_basicOptions);
//  this->clearList(m_advancedOptions);
//  this->clearList(m_newBasicOptions);
//  this->clearList(m_newAdvancedOptions);

//  m_basicOptionsNodes.clear();
//  m_advancedOptionsNodes.clear();
//  m_newBasicOptionsNodes.clear();
//  m_newAdvancedOptionsNodes.clear();

//  // set the new widgets
//  if(!options.isEmpty())
//  {
//    // get a UNIX-like path for the node
//    //   QDomNode parentNode = m_options.at(0).parentNode();
//    QString parentPath = tree->getNodePath(tree->getCurrentIndex());

//    m_gbBasicOptions->setTitle(QString("Basic options of %1").arg(parentPath));
//    m_gbAdvancedOptions->setTitle(QString("Advanced options of %1").arg(parentPath));
//    m_currentPath = parentPath;

//    // To avoid confusion, basic options panel is always showed if there is at
//    // least one option for the selected object, even if all options are advanced.
//    // Doing so, we ensure that the advanced options panel is *always* the
//    // middle one (if visible) and never the top one.
//    m_scrollBasicOptions->setVisible(true);

//    this->buttonsSetVisible(true);
//  }
//  else
//  {
//    m_scrollBasicOptions->setVisible(false);
//    m_scrollAdvancedOptions->setVisible(false);
//    this->buttonsSetVisible(false);
//  }

//  for(int i = 0 ; i < options.count() ; i++)
//  {
//    QDomNode option = options.at(i);
//    QDomNode treeOption = option.attributes().namedItem("tree");

//    // if the option "tree" attribute value is "option"
//    if(treeOption.nodeValue() == "option")
//    {
//      GraphicalOption * graphicalOption;
//      QString description;

//      QString typeString = option.attributes().namedItem("type").nodeValue();
//      OptionType::Type type = OptionType::Convert::to_enum(typeString.toStdString());

//      // if the type does not exist
//      if(type == OptionType::INVALID)
//      {
//        QString message = QString("Unknown \"%1\" type for \"%2\" option")
//        .arg(typeString)
//        .arg(option.nodeName());

//        throw UnknownTypeException(FromHere(), message.toStdString());
//      }

//      // create the graphical component
//      graphicalOption = new GraphicalOption(type);
//      graphicalOption->setName(option.nodeName() + QString(":"));
//      graphicalOption->setValue(option.toElement().text().trimmed());

//      description = option.attributes().namedItem("description").nodeValue();
//      graphicalOption->setToolTip(description);

//      // if this is a basic option...
//      if(option.attributes().namedItem("mode").nodeValue() == "basic")
//      {
//        QDomNode newNode = m_basicOptionsNodes.importNode(option, true);

//        m_basicOptions.append(graphicalOption);
//        graphicalOption->addToLayout(m_basicOptionsLayout);
//        m_basicOptionsNodes.appendChild(newNode);
//      }

//      // ...or an advanced option
//      else if(option.attributes().namedItem("mode").nodeValue() == "advanced")
//      {
//        QDomNode newNode = m_advancedOptionsNodes.importNode(option, true);

//        m_advancedOptions.append(graphicalOption);
//        graphicalOption->addToLayout(m_advancedOptionsLayout);
//        m_advancedOptionsNodes.appendChild(newNode);
//      }
//    } // "if(!childNodes.isEmpty() && treeOption.nodeValue() == "option")"
//  } // "for(int i = 0 ; i < m_options.count() ; i++)"

//  // change row stretch and panel visibilities
//  this->advancedModeChanged(tree->isAdvancedMode());
//  //  this->setAdvancedMode(this->advancedMode);

//  // set m_options to enabled or disabled (depending on their mode)
//  this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//  this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::setOptions(const QList<NodeParams> & list)
{
  QList<NodeParams>::const_iterator it = list.begin();
  const CTree::Ptr & tree = ClientRoot::getTree();

  // delete old widgets
  this->clearList(m_basicOptions);
  this->clearList(m_advancedOptions);
  this->clearList(m_newBasicOptions);
  this->clearList(m_newAdvancedOptions);

//  m_basicOptionsNodes.clear();
//  m_advancedOptionsNodes.clear();
//  m_newBasicOptionsNodes.clear();
//  m_newAdvancedOptionsNodes.clear();

  ClientRoot::getLog()->addMessage(QString("%1").arg(list.count()));

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

    this->buttonsSetVisible(true);
  }
  else
  {
    m_scrollBasicOptions->setVisible(false);
    m_scrollAdvancedOptions->setVisible(false);
    this->buttonsSetVisible(false);
  }

  while(it != list.end())
  {
    GraphicalOption * graphicalOption;

    NodeParams param = *it;
    OptionType::Type type = param.m_paramType;
    bool advanced = param.m_paramAdv;

    // create the graphical component
    try
    {
      graphicalOption = new GraphicalOption(type);
      graphicalOption->setName(param.m_paramName + ':');
      graphicalOption->setValue(param.m_paramValue.trimmed());

      graphicalOption->setToolTip(param.m_paramDescr);

      // if this is a basic option...
      if(!advanced)
      {
        //      QDomNode newNode = m_basicOptionsNodes.importNode(param, true);

        m_basicOptions.append(graphicalOption);
        graphicalOption->addToLayout(m_basicOptionsLayout);
        //      m_basicOptionsNodes.appendChild(newNode);
      }
      else     // ...or an advanced option
      {
        //      QDomNode newNode = m_advancedOptionsNodes.importNode(param, true);

        m_advancedOptions.append(graphicalOption);
        graphicalOption->addToLayout(m_advancedOptionsLayout);
        //      m_advancedOptionsNodes.appendChild(newNode);
      }

    }
    catch(UnknownTypeException ute)
    {
      ClientRoot::getLog()->addException(ute.what());
    }

    it++;

  } // end of "while()"

  // change row stretch and panel visibilities
  this->advancedModeChanged(tree->isAdvancedMode());
  //  this->setAdvancedMode(this->advancedMode);

  // set options to enabled or disabled (depending on their mode)
//  this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//  this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool OptionPanel::isModified() const
{
  bool modified = this->isModified(m_basicOptions);

  modified |= this->isModified(m_advancedOptions);
  modified |= !m_newBasicOptions.isEmpty();
  modified |= !m_newAdvancedOptions.isEmpty();

  return modified;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getModifiedOptions(CommitDetails & commitDetails) const
{
  commitDetails.clear();
  commitDetails.setNodePath(m_currentPath);

  // basic m_options
  this->getModifiedOptions(m_basicOptions, commitDetails, false);

  // advanced m_options
  this->getModifiedOptions(m_advancedOptions, commitDetails, false);

  // new basic m_options
  this->getModifiedOptions(m_newBasicOptions, commitDetails, true);

  // new advanced m_options
  this->getModifiedOptions(m_newAdvancedOptions, commitDetails, true);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString OptionPanel::getCurrentPath() const
{
  return m_currentPath;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void OptionPanel::setTreeModel(TreeModel * treeModel)
//{
//  if(treeModel != ClientRoot::getTree())
//  {
//    QModelIndex index;

//    if(ClientRoot::getTree() != CFNULL)
//      ClientRoot::getTree()->disconnect(); // disconnect all signals

//    ClientRoot::getTree() = treeModel;
//    index = treeModel->getCurrentSimulation();

//    this->advancedModeChanged(treeModel->isAdvancedMode());
//    this->readOnlyModeChanged(index, treeModel->isSimReadOnly(index));

//    if(ClientRoot::getTree() != CFNULL)
//    {
//      connect(ClientRoot::getTree(), SIGNAL(currentIndexChanged(const QModelIndex &)),
//              this, SLOT(currentIndexChanged(const QModelIndex &)));
//      connect(ClientRoot::getTree(), SIGNAL(currentSimulationChanged(const QModelIndex &)),
//              this, SLOT(currentSimulationChanged(const QModelIndex &)));
//      connect(ClientRoot::getTree(), SIGNAL(readOnlyModeChanged(const QModelIndex &, bool)),
//              this, SLOT(readOnlyModeChanged(const QModelIndex &, bool)));
//      connect(ClientRoot::getTree(), SIGNAL(advancedModeChanged(bool)),
//              this, SLOT(advancedModeChanged(bool)));
//      connect(ClientRoot::getTree(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
//              this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
//      connect(ClientRoot::getTree(), SIGNAL(simulationRemoved(const QModelIndex &)),
//              this, SLOT(simulationRemoved(const QModelIndex &)));

//      this->currentIndexChanged(ClientRoot::getTree()->getCurrentIndex());
//    }
//  }
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//TreeModel * OptionPanel::getTreeModel() const
//{
//  return ClientRoot::getTree();
//}

 // PRIVATE METHOD

QString OptionPanel::getNodePath(QDomNode & node)
{
  QDomNode parentNode = node.parentNode();
  QString name = node.nodeName();

  if(parentNode.isNull()) // if the node has no parent
    return QString();

  return this->getNodePath(parentNode) + "/" + name;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::getModifiedOptions(const QList<GraphicalOption *> & graphicalOptions,
                                     CommitDetails & commitDetails,
                                     bool newOptions) const
{
  QList<GraphicalOption *>::const_iterator it = graphicalOptions.begin();

  while(it != graphicalOptions.end())
  {

    GraphicalOption * graphicalOption = *it;

    if(graphicalOption->isModified())
    {
      QString oldValue = graphicalOption->getOrginalValueString();
      QString newValue = graphicalOption->getValueString();

      if(newOptions)
        commitDetails.setNewOption(graphicalOption->getName(), newValue, graphicalOption->getType());

      else
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

void OptionPanel::buttonsSetVisible(bool visible)
{
  m_btCommitChanges->setVisible(visible);
  m_btCheckChanges->setVisible(visible);
  m_btResetOptions->setVisible(visible);
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void OptionPanel::commitChanges() const
{
  QDomDocument modOptions = this->getOptions();
  QDomDocument newOptions = this->getNewOptions();
  QList<GraphicalOption *>::const_iterator it;

  // if there is at least one option that has been modified
  if(modOptions.hasChildNodes() || newOptions.hasChildNodes())
  {
    QModelIndex currentIndex = ClientRoot::getTree()->getCurrentIndex();

//    if(ClientRoot::getTree()->isSimulationNode(currentIndex))
//      ClientRoot::getTree()->setSimConnectionInfos(modOptions, currentIndex);
//    else
      emit changesMade(modOptions, newOptions);
  }

  it = m_basicOptions.begin();

  while(it != m_basicOptions.end())
  {
    (*it)->commit();
    it++;
  }

  it = m_advancedOptions.begin();

  while(it != m_advancedOptions.end())
  {
    (*it)->commit();
    it++;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::currentIndexChanged(const QModelIndex & index)
{
  QList<NodeParams> params;
  bool ok;

  ClientRoot::getTree()->getNodeParams(index, params, &ok);

  if(ok)
    this->setOptions(params);
  else
    ClientRoot::getLog()->addError("Invalid index!");
//  this->setOptions(ClientRoot::getTree()->getOptions(index));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void OptionPanel::currentSimulationChanged(const QModelIndex & index)
//{
//  this->setOptions();
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::advancedModeChanged(bool advanced)
{
  m_scrollAdvancedOptions->setVisible(advanced);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::dataChanged(const QModelIndex & first, const QModelIndex & last)
{
  QModelIndex currIndex = ClientRoot::getTree()->getCurrentIndex();

  if(first == last && first.row() == currIndex.row() && first.parent() == currIndex.parent())
    this->currentIndexChanged(first);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::readOnlyModeChanged(const QModelIndex & index, bool readOnly)
{
  // if the parameter and the attribute are different...
  if(m_readOnly ^ readOnly /*&& ClientRoot::getTree()->isCurrentSimIndex(index)*/)
  {
    m_readOnly = readOnly;

    // ...we change the editors readOnly property
//    this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//    this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void OptionPanel::simulationRemoved(const QModelIndex & index)
//{
//  if(index == ClientRoot::getTree()->getCurrentSimulation())
//    this->setOptions(QDomNodeList());
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::checkOptions()
{
  CommitDetails details;
  CommitDetailsDialog dialog;

  this->getModifiedOptions(details);
  dialog.show(details);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void OptionPanel::resetChanges()
{
  this->currentIndexChanged(ClientRoot::getTree()->getCurrentIndex());
}
