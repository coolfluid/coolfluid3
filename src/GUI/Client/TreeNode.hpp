#ifndef CF_GUI_Client_TreeNode_hpp
#define CF_GUI_Client_TreeNode_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "GUI/Client/CNode.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  class TreeNode
  {
  public:

    TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber);

    bool hasParent() const;

    TreeNode * getChild(int index);

    CNode::Ptr getNode() const;

    TreeNode * getParent() const;

    int getRowNumber() const;

    int getChildCount() const;

    QString getName() const;

    TreeNode * getChildByName(const QString & name);

  private:

    CNode::Ptr m_node;

    TreeNode * m_parent;

    int m_rowNumber;

    QList<TreeNode *> m_childNodes;

  }; // class MyTreeItem

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_TreeNode_hpp
