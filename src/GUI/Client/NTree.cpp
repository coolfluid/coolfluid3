#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"
#include "GUI/Client/NRoot.hpp"
#include "GUI/Client/TreeNode.hpp"
#include "GUI/Client/NLink.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NTree.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

NTree::NTree(CNode::Ptr rootNode)
  : CNode(CLIENT_TREE, "NTree", CNode::TREE_NODE),
    m_advancedMode(false)
{
  BUILD_COMPONENT;

//  cf_assert(rootNode.get() != CFNULL);

  if(rootNode.get() == CFNULL)
  {
//    QString data =
//        "<CRoot name=\"Simulation\" >"
//        " <CGroup name=\"Flow\" >"
//        "  <CLink name=\"Mesh\">//Simulation/MG/Mesh1</CLink>"
//        "  <CMethod name=\"FVM\" >"
//        "   <params>"
//        "    <int key=\"iter\" mode=\"basic\" desc=\"nb iterations\" >5</int>"
//        "    <string key=\"somename\" mode=\"adv\" >Lolo</string>"
//        "    <path   key=\"region\">./</path>"
//        "   </params>"
//        "  </CMethod>"
//        "  <CMethod name=\"Petsc\" >"
//        "    <params>"
//        "     <int key=\"iter2\" mode=\"basic\" desc=\"nb iterations\" >5</int>"
//        "     <string key=\"somename2\" mode=\"adv\" >Lolo</string>"
//        "     <path   key=\"region2\">./</path>"
//        "    </params>"
//        "  </CMethod>"
//        " </CGroup>"
//        " <CGroup name=\"MG\">"
//        "  <params>"
//        "   <bool key=\"myBool\" mode=\"basic\" desc=\"a boolean option\" >true</bool>"
//        "   <string key=\"someOtherName\" mode=\"adv\" >Lolo</string>"
//        "  </params>"
//        "  <CMesh name=\"Mesh1\" >"
//        "   <params>"
//        "    <bool key=\"myOtherBool\" mode=\"basic\" desc=\"another boolean option\" >false</bool>"
//        "    <string key=\"yetAnotherName\" mode=\"adv\" >Lolo</string>"
//        "   </params>"
//        "  </CMesh>"
//        "  <CMesh name=\"Mesh2\" > <!-- mesh1 here --> </CMesh>"
//        " </CGroup>"
//        " <CGroup name=\"Solid\">"
//        "  <CMesh name=\"Mesh3\" > <!-- mesh2 here --> </CMesh>"
//        "  <CMesh name=\"Mesh4\" > <!-- mesh1 here --> </CMesh>"
//        "  <CLink name=\"PetscLink\">//Simulation/Flow/Petsc</CLink>"
//        " </CGroup>"
//        "</CRoot>";
//    QDomDocument doc;

//    doc.setContent(data);

//    CNode::Ptr nodePtr = CNode::createFromXml(doc.firstChildElement());

    m_rootNode = new TreeNode(ClientRoot::getRoot(), CFNULL, 0);
  }

//  m_rootNode = CFNULL;

  m_rootNode = new TreeNode(rootNode, CFNULL, 0);

  m_columns << "Name" << "Type";

  regist_signal("list_tree", "New tree")->connect(boost::bind(&NTree::list_tree, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setRoot(NRoot::Ptr rootNode)
{
//  cf_assert(rootNode.get() != CFNULL);

  // initiate the removing process
  emit layoutAboutToBeChanged();
//  this->beginRemoveRows(QModelIndex(), 0, m_rootNode->getChildCount() - 1);
  delete m_rootNode;
//  this->endRemoveRows(); // end the removing process

  m_rootNode = CFNULL;

  if(rootNode.get() != CFNULL)
  {
//    QString data =
//        "<CRoot name=\"Simulation\" >"
//        " <CGroup name=\"Flow\" >"
//        "  <CLink name=\"Mesh\">//Simulation/MG/Mesh1</CLink>"
//        "  <CMethod name=\"FVM\" >"
//        "   <params>"
//        "    <int key=\"iter\" mode=\"basic\" desc=\"nb iterations\" >5</int>"
//        "    <string key=\"somename\" mode=\"adv\" >Lolo</string>"
//        "    <path   key=\"region\">./</path>"
//        "   </params>"
//        "  </CMethod>"
//        "  <CMethod name=\"Petsc\" >"
//        "    <params>"
//        "     <int key=\"iter2\" mode=\"basic\" desc=\"nb iterations\" >5</int>"
//        "     <string key=\"somename2\" mode=\"adv\" >Lolo</string>"
//        "     <path   key=\"region2\">./</path>"
//        "    </params>"
//        "  </CMethod>"
//        " </CGroup>"
//        " <CGroup name=\"MG\">"
//        "  <params>"
//        "   <bool key=\"myBool\" mode=\"basic\" desc=\"a boolean option\" >true</bool>"
//        "   <string key=\"someOtherName\" mode=\"adv\" >Lolo</string>"
//        "  </params>"
//        "  <CMesh name=\"Mesh1\" >"
//        "   <params>"
//        "    <bool key=\"myOtherBool\" mode=\"basic\" desc=\"another boolean option\" >false</bool>"
//        "    <string key=\"yetAnotherName\" mode=\"adv\" >Lolo</string>"
//        "   </params>"
//        "  </CMesh>"
//        "  <CMesh name=\"Mesh2\" > <!-- mesh1 here --> </CMesh>"
//        " </CGroup>"
//        " <CGroup name=\"Solid\">"
//        "  <CMesh name=\"Mesh3\" > <!-- mesh2 here --> </CMesh>"
//        "  <CMesh name=\"Mesh4\" > <!-- mesh1 here --> </CMesh>"
//        "  <CLink name=\"PetscLink\">//Simulation/Flow/Petsc</CLink>"
//        " </CGroup>"
//        "</CRoot>";
//    QDomDocument doc;

//    doc.setContent(data);

//    CNode::Ptr nodePtr = CNode::createFromXml(doc.firstChildElement());

//    ComponentIterator<CNode> it = rootNode->begin<CNode>();

//    rootNode->rename(nodePtr->name());
//    rootNode->root()->rename(nodePtr->name());

//    while(it != nodePtr->end<CNode>())
//    {
//      rootNode->root()->add_component(it.get());
//      it++;
//    }

    m_rootNode = new TreeNode(rootNode, CFNULL, 0);

    emit layoutChanged();
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setCurrentIndex(const QModelIndex & newIndex)
{
  if(!this->areFromSameNode(m_currentIndex, newIndex))
  {
    QModelIndex oldIndex = m_currentIndex;
    m_currentIndex = newIndex;
    emit currentIndexChanged(newIndex, oldIndex);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::getCurrentIndex() const
{
  return m_currentIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getNodeParams(const QModelIndex & index, QList<NodeOption> & params,
                          bool * ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != CFNULL)
    *ok = node != CFNULL;

  params.clear();

  if(node != CFNULL)
    node->getNode()->getOptions(params);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTree::getNodePath(const QModelIndex & index) const
{
  QString path;

  this->getNodePathRec(index, path);

  return path;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::setAdvancedMode(bool advanceMode)
{
  if(m_advancedMode != advanceMode)
  {
    m_advancedMode = advanceMode;
    emit advancedModeChanged(m_advancedMode);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::isAdvancedMode() const
{
  return m_advancedMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::areFromSameNode(const QModelIndex & left, const QModelIndex & right) const
{
  return left.isValid() && left.internalPointer() == right.internalPointer();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NTree::haveSameData(const QModelIndex & left, const QModelIndex & right) const
{
  bool sameData = false;
  TreeNode * leftTreeNode = this->indexToTreeNode(left);
  TreeNode * rightTreeNode = this->indexToTreeNode(right);

  if(leftTreeNode != CFNULL && rightTreeNode != CFNULL)
  {
    CNode::Ptr leftNode = leftTreeNode->getNode();
    CNode::Ptr rightNode = rightTreeNode->getNode();

    if(leftNode->checkType(CNode::LINK_NODE))
    {
      sameData = CNode::convertTo<NLink>(leftNode)->getTargetPath().string() == QString("//%1").arg(rightNode->full_path().string().c_str()).toStdString();
    }
    else if(rightNode->checkType(CNode::LINK_NODE))
    {
      sameData = CNode::convertTo<NLink>(rightNode)->getTargetPath().string() == QString("//%1").arg(leftNode->full_path().string().c_str()).toStdString();
    }
  }

  return sameData;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CNode::Ptr NTree::getNodeByPath(const CPath & path) const
{
  QString pathStr = path.string().c_str();
  QStringList comps;
  QStringList::iterator it;
  CNode::Ptr node = m_rootNode->getNode();

  if(!path.is_absolute())
    ClientRoot::getLog()->addError(QString("\"%1\" is not an absolute path").arg(pathStr));
  else
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first().toStdString() == node->name())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && node.get() != CFNULL ; it++)
    {
      if(node->checkType(CNode::ROOT_NODE))
        node = boost::dynamic_pointer_cast<CNode>(CNode::convertTo<NRoot>(node)->root()->get_child(it->toStdString()));
      else
        node = boost::dynamic_pointer_cast<CNode>(node->get_child(it->toStdString()));
    }
  }

  return node;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::getIndexByPath(const CPath & path) const
{
  QModelIndex index;
  QString pathStr = path.string().c_str();
  QStringList comps;
  QStringList::iterator it;
  TreeNode * treeNode = m_rootNode;

  cf_assert(treeNode != CFNULL);

  /// @todo find a better algorithm !!!
  if(!path.is_absolute())
    ClientRoot::getLog()->addError(QString("\"%1\" is not an absolute path").arg(pathStr));
  else
  {
    comps = pathStr.split(CPath::separator().c_str(), QString::SkipEmptyParts);

    if(comps.first() == treeNode->getName())
      comps.removeFirst();

    for(it = comps.begin() ; it != comps.end() && treeNode != CFNULL ; it++)
    {
      treeNode = treeNode->getChildByName(*it);
      index = this->index(treeNode->getRowNumber(), 0, index);
    }

  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NTree::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid())
  {
    CNode::Ptr node = this->indexToNode(index);

    if(role == Qt::DisplayRole)
    {
      switch(index.column())
      {
      case 0:
        data = QString("%1").arg(node->name().c_str());
        break;
      case 1:
        data = QString("%1").arg(node->getComponentType());
        break;
      }
    }
    else
    {
      if(role == Qt::DecorationRole && index.column() == 0)
        data = node->getIcon();

      if(role == Qt::ToolTipRole)
        data = node->getToolTip();
    }
  }

  return data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::index(int row, int column, const QModelIndex & parent) const
{
  TreeNode * childNode;
  TreeNode * parentNode;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if( !parent.isValid())
      childNode = m_rootNode;
    else
      childNode = this->indexToTreeNode(parent)->getChild(row);

    if(childNode != CFNULL)
      index = createIndex(row, column, childNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex NTree::parent(const QModelIndex &child) const
{
  QModelIndex index;

  if(child.isValid())
  {
    TreeNode * childNode = this->indexToTreeNode(child);
    TreeNode * parentNode = childNode->getParent();

    if (parentNode != CFNULL)
      index = createIndex(parentNode->getRowNumber(), 0, parentNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NTree::rowCount(const QModelIndex & parent) const
{
  if (parent.column() > 0)
    return 0;

  // if the parent is not valid, we have one child: the root
  if (!parent.isValid())
    return 1;

  return this->indexToTreeNode(parent)->getChildCount();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int NTree::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant NTree::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::showNodeMenu(const QModelIndex & index, const QPoint & pos) const
{
  TreeNode * treeNode = indexToTreeNode(index);

  cf_assert(treeNode != CFNULL);

  if(treeNode != CFNULL)
    treeNode->getNode()->showContextMenu(pos);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CF::Common::Signal::return_t NTree::list_tree(CF::Common::Signal::arg_t & node)
{
//  ComponentIterator<CNode> it = rootNode->begin<CNode>();

//  rootNode->rename(nodePtr->name());
//  rootNode->root()->rename(nodePtr->name());

//  while(it != nodePtr->end<CNode>())
//  {
//    rootNode->root()->add_component(it.get());
//    it++;
//  }
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

Signal::return_t NTree::updateTree(Signal::arg_t & node)
{
  std::string str;
  QDomDocument doc;

  XmlOps::xml_to_string(node, str);

  doc.setContent(QString(str.c_str()));

  try
  {
    m_rootNode = new TreeNode(CNode::createFromXml(doc.firstChildElement()), CFNULL, 0);
  }
  catch(XmlError xe)
  {
    ClientRoot::getLog()->addException(xe.what());
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getNodePathRec(const QModelIndex & index, QString & path) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != CFNULL)
  {
    path.prepend('/').prepend(node->getName());
    this->getNodePathRec(index.parent(), path);
  }
  else
    path.prepend("//");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NTree::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NTree::getToolTip() const
{
  return this->getComponentType();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NTree::getOptions(QList<NodeOption> & params) const
{
  params = m_options;
}
