#ifndef CF_GUI_Client_NTree_hpp
#define CF_GUI_Client_NTree_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QHash>
#include <QStringList> /// @todo does not compile without that...but why ???

#include "GUI/Client/CNode.hpp"
#include "GUI/Client/TreeNode.hpp"
#include "GUI/Client/NRoot.hpp"

class QDomElement;
class QModelIndex;

namespace CF {

namespace Common { class CPath; }

namespace GUI {
namespace Client {

  ///////////////////////////////////////////////////////////////////////////

  class TreeNode;

  /// @brief Tree model

  class NTree :
      public QAbstractItemModel,
      public CNode
  {
    Q_OBJECT
  public:

    typedef boost::shared_ptr<NTree> Ptr;
    typedef boost::shared_ptr<NTree const> ConstPtr;

    /// @brief Constructor.

    /// @param rootNode The root node. May be @c CFNULL.
    NTree(CF::GUI::Client::CNode::Ptr rootNode = CF::GUI::Client::CNode::Ptr());

    /// @brief Replaces the current component tree.

    /// The old tree is destroyed (regarding to @c boost::shared_ptr delete
    /// rules).
    /// @param node The new root. May be @c CFNULL.
    void setRoot(NRoot::Ptr node);

    /// @brief Sets the current index.

    /// If @c newIndex is valid and different from the current index, the
    /// current index is changed and #currentIndexChanged signal is emitted.
    /// If @c newIndex is either not valid or the same as the current index,
    /// nothing is done.
    /// @param newIndex The new index.
    /// @see getCurrentIndex
    /// @see currentIndexChanged
    void setCurrentIndex(const QModelIndex & newIndex);

    /// @brief Gives the current index

    /// The current index may not be valid if @c #changeCurrentIndex was
    /// never called.
    /// @return Returns the current index.
    /// @see setCurrentIndex.
    QModelIndex getCurrentIndex() const;

    /// @brief Gets node options

    /// @param index Node index
    /// @param params List where options will be stored
    /// @param ok If not @c CFNULL, used to strore whether the option
    /// gathering succeded or not.
    void getNodeParams(const QModelIndex & index,
                       QList<NodeOption> & params, bool * ok = CFNULL) const;

    /// @brief Retrieves a node path.

    /// @param index Node index
    /// @return Returns the node path
    QString getNodePath(const QModelIndex & index) const;

    /// @brief Set advanced mode

    /// @param advancedMode If @c true, advanced mode is activated.
    void setAdvancedMode(bool advanceMode);

    /// @brief Indicates whether advanced mode is activated or not.

    /// @return Returns @c true if advanced mode is activated, otherwise,
    /// returns @c false.
    bool isAdvancedMode() const;

    /// @brief Checks whether two indexes point to the same node.

    /// If indexes point to a null node, they are considered as not
    /// pointing to the same node.
    /// @param left Left node
    /// @param right Right node
    /// @return Returns @c true if both indexes point to the same node.
    /// Otherwise returns @c false.
    bool areFromSameNode(const QModelIndex & left, const QModelIndex & right) const;

    /// @brief Checks whether two indexes have the same data.

    /// This method can be used to check if one link node has the other
    /// one as target.
    /// @param left Left index.
    /// @param right Right index.
    /// @return Returns @c true if both node have the same data. Otherwise,
    /// returns @c false.
    bool haveSameData(const QModelIndex & left, const QModelIndex & right) const;

    /// @brief Retrieves a node from its path.

    /// @param path The node path
    /// @return Returns the found node, or a null shared pointer if
    /// the node does not exist.
    CNode::Ptr getNodeByPath(const CF::Common::CPath & path) const;

    /// @brief Retrieves a node index from its path.

    /// @param path The node index path
    /// @return Returns the found node index, or a invalid index if
    /// it does not exist.
    QModelIndex getIndexByPath(const CF::Common::CPath & path) const;

    /// @brief Retrieves an index frome a node

    /// @param node The node
    /// @return Returns the found index, or a invalid index if
    /// it does not exist.
    QModelIndex nodeToIndex(const CNode::Ptr & node) const;

    /// @brief Modifies options of a node

    /// This method calls @c CNode::modifyOptions() of the corresponding
    /// node
    /// @param index Node index
    /// @param options Options to modify. The key is the option name and
    /// the value is the option value to set.
    void modifyOptions(const QModelIndex & index,
                       const QHash<QString, QString> & options);

    /// @brief Implementation of @c QAbstractItemModel::data().

    /// Only the role @c Qt::DisplayRole and @c Qt::DecorationRole are accepted.
    /// Other roles will result to the return of an empty @c QVariant object
    /// (built with the default construtor).
    /// @param index Concerned item index.
    /// @param role Role of the returned value (only @c Qt::DisplayRole or
    /// @c Qt::DecorationRole).
    /// @return Returns an empty QVariant object if the role is neither
    /// @c Qt::DisplayRole nor @c Qt::DecorationRole or if the @c index.isValid()
    /// returns @c false. Otherwise, returns the nodename of the
    /// the item at the specified index.
    virtual QVariant data(const QModelIndex & index, int role) const;

    /// @brief Implementation of @c QAbstractItemModel::index().

    /// Gives the index of the item at the given row and column under
    /// the given parent. If the parent index is not valid, the root item
    /// is taken as parent.
    /// @param row Item row from the parent.
    /// @param column Item column.
    /// @param parent Item parent.
    /// @return Returns the requested index, or a CFNULL index if
    /// <code>hasIndex(row, column, parent)</code> returns @c false.
    virtual QModelIndex index(int row, int column,
                              const QModelIndex & parent = QModelIndex()) const;

    /// @brief Implementation of @c QAbstractItemModel::parent().

    /// @param child Item index of which we would like to know the parent.
    /// @return Returns the parent index of the given child or a CFNULL
    /// index if the child is not a valid index.
    virtual QModelIndex parent(const QModelIndex &child) const;

    /// @brief Implementation of @c QAbstractItemModel::rowCount().

    /// If the parent index is not valid, the root item is taken as parent.
    /// @return Returns the row count (number of children) of a given parent.
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /// @brief Implementation of @c QAbstractItemModel::columnCount().
    /// @return Always returns 1.
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    /// @brief Gives header titles.

    /// Overrides @c QAbstractItemModel::headerData().
    /// @param section Section number.
    /// @param orientation Header orientation.
    /// @param role Data role. Only @c Qt::DisplayRole is accepted.
    /// @return Returns the data or an empty @c QVariant on error.
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    /// @brief Shows the context menu

    /// @param index Node index
    /// @param pos Position of the top-left corner of the menu.
    void showNodeMenu(const QModelIndex & index, const QPoint & pos) const;

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    /// @brief Gives the tool tip text
    /// @return Returns The class name
    virtual QString getToolTip() const;

    /// @brief Set the debug mode

    /// In debug mode, client components are showed.
    /// @param debugMode If @c true, the debug mode is activated. Otherwise,
    /// it is deactivated.
    void setDebugModeEnabled(bool debugMode);

    /// @brief Indicates whether the debug mode is activated or not.

    /// @return Returns @c true if the debug mode is activated; otherwise,
    /// returns @c false.
    bool isDebugModeEnabled() const;

    /// @name Signals
    /// @{

    /// @brief Signal called when the tree needs to be updated

    /// @param node New tree
    void list_tree(CF::Common::XmlNode & node);

    /// @} END Signals

  signals:

    /// @brief Signal emitted when the current index has changed.

    /// @param newIndex The new current index
    /// @param oldIndex The old current index
    /// @see setCurrentIndex
    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    /// @brief Signal emitted when advanced mode changed

    /// @param advanced New advanced mode status
    /// @see setAdvancedMode
    void advancedModeChanged(bool advanced);

  private:

    /// @brief Column titles
    QStringList m_columns;

    /// @brief The root node
    TreeNode * m_rootNode;

    /// @brief Current index
    QModelIndex m_currentIndex;

    /// @brief Indicates whether we are in advanced mode or not
    bool m_advancedMode;

    /// @brief Indicates whether we are in debug mode or not
    bool m_debugModeEnabled;

    /// @brief Converts an index to a tree node

    /// @param index Node index to convert
    /// @return Returns the tree node, or @c CFNULL if the index could
    /// not be converted (i.e. index is invalid)
    inline TreeNode * indexToTreeNode(const QModelIndex & index) const
    {
      return static_cast<TreeNode *>(index.internalPointer());
    }

    /// @brief Converts an index to a node

    /// @param index Node index to convert
    /// @return Returns the node, or a null shared pointer if the index could
    /// not be converted (i.e. index is invalid)
    inline CNode::Ptr indexToNode(const QModelIndex & index) const
    {
      return this->indexToTreeNode(index)->getNode();
    }

    /// @brief Retrieves a node path from its index.

    /// This is a recursive method.
    /// @param index Node index.
    /// @param path Intermediate retrieved path
    void getNodePathRec(const QModelIndex & index, QString & path) const;


    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}


  }; // class NTree

  ///////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_NTree_hpp
