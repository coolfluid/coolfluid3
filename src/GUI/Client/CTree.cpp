#include <QtCore>
#include <QtGui>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/CNode.hpp"
#include "GUI/Client/TreeNode.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/CTree.hpp"

//#define TO_CNODE_PTR(a) boost::dynamic_pointer_cast<CNode>(a).get()
//#define this->indexToTreeNode(a) static_cast<TreeNode *>(a.internalPointer())
//#define this->indexToNode(a) this->indexToTreeNode(a)->getNode()

using namespace CF::Common;
using namespace CF::GUI::Client;

CTree::CTree(CNode::Ptr rootNode)
  : Component(CLIENT_TREE),
    m_advancedMode(false)
{
//  cf_assert(rootNode.get() != CFNULL);

  if(rootNode.get() == CFNULL)
  {
    QString data =
        "<CRoot name=\"Simulation\" >"
        " <CGroup name=\"Flow\" >"
        "  <CLink name=\"Mesh\">//Simulation/MG/Mesh1</CLink>"
        "  <CMethod name=\"FVM\" >"
        "   <params>"
        "    <int key=\"iter\" mode=\"basic\" desc=\"nb iterations\" >5</int>"
        "    <string key=\"somename\" mode=\"adv\" >Lolo</string>"
        "    <path   key=\"region\">./</path>"
        "   </params>"
        "  </CMethod>"
        "  <CMethod name=\"Petsc\" > <!-- petsc here --> </CMethod>"
        " </CGroup>"
        " <CGroup name=\"MG\">"
        "  <params>"
        "   <bool key=\"myBool\" mode=\"basic\" desc=\"a boolean option\" >true</bool>"
        "   <string key=\"someOtherName\" mode=\"adv\" >Lolo</string>"
        "  </params>"
        "  <CMesh name=\"Mesh1\" >"
        "   <params>"
        "    <bool key=\"myOtherBool\" mode=\"basic\" desc=\"another boolean option\" >false</bool>"
        "    <string key=\"yetAnotherName\" mode=\"adv\" >Lolo</string>"
        "   </params>"
        "  </CMesh>"
        "  <CMesh name=\"Mesh2\" > <!-- mesh1 here --> </CMesh>"
        " </CGroup>"
        " <CGroup name=\"Solid\">"
        "  <CMesh name=\"Mesh3\" > <!-- mesh2 here --> </CMesh>"
        "  <CMesh name=\"Mesh4\" > <!-- mesh1 here --> </CMesh>"
        "  <CLink name=\"PestcLink\">//Simulation/Flow/Pestc</CLink>"
        " </CGroup>"
        "</CRoot>";
    QDomDocument doc;

    doc.setContent(data);

    CNode::Ptr nodePtr = CNode::createFromXml(doc.firstChildElement());

    m_rootNode = new TreeNode(nodePtr, CFNULL, 0);
  }
  else
    m_rootNode = new TreeNode(rootNode, CFNULL, 0);

  m_columns << "Name" << "Type";

  regist_signal("list_tree", "Log message")->connect(boost::bind(&CTree::list_tree, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CTree::setRoot(CNode::Ptr rootNode)
{
  cf_assert(rootNode.get() != CFNULL);

  // initiate the removing process
  this->beginRemoveRows(QModelIndex(), 0, m_rootNode->getChildCount() - 1);
  delete m_rootNode;
  this->endRemoveRows(); // end the removing process

  m_rootNode = new TreeNode(rootNode, CFNULL, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CTree::setCurrentIndex(const QModelIndex & newIndex)
{
  if(!this->areFromSameNode(m_currentIndex, newIndex))
  {
    m_currentIndex = newIndex;
    emit currentIndexChanged(newIndex);
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex CTree::getCurrentIndex() const
{
  return m_currentIndex;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CTree::getNodeParams(const QModelIndex & index, QList<NodeParams> & params,
                          bool * ok) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(ok != CFNULL)
    *ok = node != CFNULL;

  params.clear();

  if(node != CFNULL)
    node->getNode()->getParams(params);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString CTree::getNodePath(const QModelIndex & index) const
{
  QString path;

  this->getNodePathRec(index, path);

  return path;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CTree::setAdvancedMode(bool advanceMode)
{
  if(m_advancedMode != advanceMode)
  {
    m_advancedMode = advanceMode;
    emit advancedModeChanged(m_advancedMode);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CTree::isAdvancedMode() const
{
  return m_advancedMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CTree::areFromSameNode(const QModelIndex & left, const QModelIndex & right) const
{
  return left.isValid() && left.internalPointer() == right.internalPointer();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant CTree::data(const QModelIndex & index, int role) const
{
  QVariant data;

  if(index.isValid())
  {
    CNode::Ptr node = this->indexToNode(index);
    QString parent;

    if(index.parent().isValid())
      parent = this->indexToNode(index.parent())->name().c_str();

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

QModelIndex CTree::index(int row, int column, const QModelIndex & parent) const
{
  TreeNode * childNode;
  TreeNode * parentNode;
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if(!parent.isValid())
      parentNode = m_rootNode;
    else
      parentNode = this->indexToTreeNode(parent);

    childNode = parentNode->getChild(row);

    if(childNode != CFNULL)
      index = createIndex(row, column, childNode);
    else
      index = QModelIndex();
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QModelIndex CTree::parent(const QModelIndex &child) const
{
  QModelIndex index;

  if(child.isValid())
  {
    TreeNode * childNode = this->indexToTreeNode(child);
    TreeNode * parentNode = childNode->getParent();

    if (parentNode != CFNULL && parentNode != m_rootNode)
      index = createIndex(parentNode->getRowNumber(), 0, parentNode);
  }

  return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CTree::rowCount(const QModelIndex & parent) const
{
  TreeNode * parentItem;

  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = m_rootNode;
  else
    parentItem = this->indexToTreeNode(parent);

  return parentItem->getNode()->getNodeCount();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int CTree::columnCount(const QModelIndex & parent) const
{
  return m_columns.count();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QVariant CTree::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0
     && section < m_columns.size())
    return m_columns.at(section);

  return QVariant();
}

/*============================================================================

                             PRIVATE METHODS

============================================================================*/

Signal::return_t CTree::updateTree(Signal::arg_t & node)
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

inline TreeNode * CTree::indexToTreeNode(const QModelIndex & index) const
{
  return static_cast<TreeNode *>(index.internalPointer());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline CNode::Ptr CTree::indexToNode(const QModelIndex & index) const
{
  return this->indexToTreeNode(index)->getNode();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CTree::getNodePathRec(const QModelIndex & index, QString & path) const
{
  TreeNode * node = this->indexToTreeNode(index);

  if(node != CFNULL)
  {
    path.prepend('/').prepend(node->getName());
    this->getNodePathRec(index.parent(), path);
  }
}
