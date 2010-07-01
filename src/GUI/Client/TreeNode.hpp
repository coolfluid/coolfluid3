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

  /// @brief Handles a CNode component in the tree.

  class TreeNode
  {
  public:

    /// @brief Constructor.

    /// @param node The node to handle. The pointer can not be @c CFNULL.
    /// @param parent Pointer to the parent TreeNode. May be @c CFNULL.
    /// @param rowNumber Row number of the node under the parent. Must be
    /// greater or equal to 0.
    TreeNode(CNode::Ptr node, TreeNode * parent, int rowNumber);

    /// @brief Checks whether the node has a parent or not.

    /// A node has a parent if was constructed with a non-null pointer as
    /// @c parent.
    /// @return Returns @c true if the node has parent. Otherwise, returns
    /// @c false.
    bool hasParent() const;

    /// @brief Returns the @e ith child of this node.

    /// If the TreeNode object corresponding to the asked child does not exist
    /// yet, it is created.
    /// @param rowNumber
    /// @return Returns the wanted child, or a @c CFNULL pointer if the row
    /// number is not valid (less than 0, or bigger than the number of
    /// child this this node has).
    TreeNode * getChild(int rowNumber);

    /// @brief Gives the node handled by this object
    /// @returns Returns the node handled by this object.
    CNode::Ptr getNode() const;

    /// @brief Gives the parent.
    /// @return Returns the parent. May return a @c CFNULL pointer if the
    /// node has no object.
    TreeNode * getParent() const;

    /// @brief Gives the row number.
    /// @return Returns the row number.
    int getRowNumber() const;

    /// @brief Gives the child count.
    /// @return Returns the child count.
    int getChildCount() const;

    /// @brief Gives the node name.

    /// Calling the method is equivalent to
    /// @code node->getNode()->name().str(); @endcode
    /// @return Return the node name.
    inline QString getName() const
    {
      return m_node->name().c_str();
    }

    /// @brief Retrieves a child from its name.

    /// @return Returns the child, or a @c CFNULL pointer if no child
    /// as such name.
    TreeNode * getChildByName(const QString & name);

  private:

    /// @brief Handled node.
    CNode::Ptr m_node;

    /// @brief The parent. May be @c CFNULL
    TreeNode * m_parent;

    /// @brief The row number.
    int m_rowNumber;

    /// @brief List of children.

    /// This list is initialized at the right size in the constructor with
    /// @c CFNULL pointers (one pointer for each child). Each pointer is
    /// replaced when the corresponding child is built by @c getChild method.
    /// The size of this list is never modified.
    QList<TreeNode *> m_childNodes;

  }; // class MyTreeItem

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_TreeNode_hpp
