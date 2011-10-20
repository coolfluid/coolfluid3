// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_NTree_hpp
#define cf3_GUI_Core_NTree_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QMap>
#include <QStringList>

#include "UI/Core/CNode.hpp"
#include "UI/Core/NRoot.hpp"

class QRegExp;

template<typename T> class QList;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { class URI; }

namespace UI {
namespace Core {

  ///////////////////////////////////////////////////////////////////////////

  class TreeNode;

  /// @brief Tree model
  /// @author Quentin Gasper.

  class Core_API NTree :
      public QAbstractItemModel,
      public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NTree> Ptr;
    typedef boost::shared_ptr<NTree const> ConstPtr;

    /// @brief Constructor.

    /// If the root node is a @c nullptr pointer
    /// @param rootNode The root node. May be @c nullptr.
    NTree(cf3::UI::Core::NRoot::Ptr rootNode = cf3::UI::Core::NRoot::Ptr());

    /// @brief Replaces the current component tree.

    /// The old tree is destroyed (regarding to @c boost::shared_ptr delete
    /// rules).
    /// @param node The new root. May be @c nullptr.
    void setRoot(NRoot::Ptr node);

    /// @brief Gives the current root
    /// @return Returns the current root
    NRoot::Ptr treeRoot() const;

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
    QModelIndex currentIndex() const;

    /// @brief Gives the path of the current index.
    /// @return Returns the path of the index returned by @c #getCurrentIndex()
    /// or an empty path if not valid current index is set.
    cf3::common::URI currentPath() const;

    /// @brief Gets node options

    /// @param index Node index
    /// @param options List where options will be stored
    /// @param ok If not @c nullptr, used to strore whether the option
    /// gathering succeded or not.
    void listNodeOptions(const QModelIndex & index,
                         QList<cf3::common::Option::ConstPtr> & options,
                         bool * ok = nullptr) const;

    /// @brief Gets node properties

    /// @param index Node index
    /// @param props Map where properties will be stored. The key is the
    /// property name, the value is the property value.
    /// @param ok If not @c nullptr, used to store whether the property
    /// gathering succeeded or not.
    void listNodeProperties(const QModelIndex & index,
                            QMap<QString, QString> & params,
                            bool * ok = nullptr) const;

    /// @brief Gets node actions

    /// @param index Node index
    /// @param actions List where action will be stored.
    /// @param ok If not @c nullptr, used to store whether the action
    /// gathering succeeded or not (i.e. it fails if the index is not valid).
    void listNodeActions(const QModelIndex & index, QList<ActionInfo> & actions,
                         bool * ok = nullptr) const;

    /// @brief Retrieves a node path.

    /// @param index Node index
    /// @return Returns the node path
    QString nodePath(const QModelIndex & index) const;

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

    /// @brief Retrieves a node from its path.

    /// @param path The node path
    /// @return Returns the found node, or a null shared pointer if
    /// the node does not exist.
    CNode::ConstPtr nodeByPath(const cf3::common::URI & path) const;

    /// @brief Retrieves a node from its path.

    /// @param path The node path
    /// @return Returns the found node, or a null shared pointer if
    /// the node does not exist.
    CNode::Ptr nodeByPath(const cf3::common::URI & path);

    /// @brief Retrieves a node index from its path.

    /// @param path The node index path
    /// @return Returns the found node index, or a invalid index if
    /// it does not exist.
    QModelIndex indexFromPath(const cf3::common::URI & path) const;

    /// @brief Gives the path of the provided index.
    /// @param index Index of which we want to know the path.
    /// @return Returns the index path, or an empty path if the index is not
    /// valid.
    cf3::common::URI pathFromIndex(const QModelIndex & index) const;

    /// @brief Modifies options of a node

    /// This method calls @c CNode::modifyOptions() of the corresponding
    /// node
    /// @param index Node index
    /// @param options Options to modify. The key is the option name and
    /// the value is the option value to set.
    void modifyOptions(const QModelIndex & index,
                       const QMap<QString, QString> & options);

    /// @name VIRTUAL FUNCTIONS
    //@{

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
    /// @return Returns the requested index, or a nullptr index if
    /// <code>hasIndex(row, column, parent)</code> returns @c false.
    virtual QModelIndex index(int row, int column,
                              const QModelIndex & parent = QModelIndex()) const;

    /// @brief Implementation of @c QAbstractItemModel::parent().

    /// @param child Item index of which we would like to know the parent.
    /// @return Returns the parent index of the given child or a nullptr
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

    /// @brief Gives the tool tip text
    /// @return Returns The class name
    virtual QString toolTip() const;

    //@} END VIRTUAL FUNCTIONS

    /// @brief Set the debug mode

    /// In debug mode, client components are showed.
    /// @param debugMode If @c true, the debug mode is activated. Otherwise,
    /// it is deactivated.
    void setDebugModeEnabled(bool debugMode);

    /// @brief Indicates whether the debug mode is activated or not.

    /// @return Returns @c true if the debug mode is activated; otherwise,
    /// returns @c false.
    bool isDebugModeEnabled() const;

    /// @brief Updates the children row counts, starting from the root.
    /// @note This method emits a @c layoutChanged() signal, causing the
    /// view(s) to be completely updated. Calling this method too often might
    /// decrease the program performances.
    void updateRootChildren();

    void optionsChanged(const cf3::common::URI & path);

    /// @brief Checks whether a node name or one of its children matches a
    /// provided regular expression.
    /// @param index Index of the node to check.
    /// @param regex Regular expression to match.
    /// @return Returns @c true if the node or at least one of its children
    /// matches the regular expression.
    bool nodeMatches(const QModelIndex & index, const QRegExp & regex) const;

    /// @brief Checks whether a nodeis visible or not.

    /// A node is visible if:
    /// @li it is basic
    /// @li it is advanced @b and advanced mode is activated
    /// @li it is a client component @b and debug mode is activated.
    /// @param index Index of the node to check
    /// @return Returns @c true if the node is visible. Otherwise, returns
    /// @c false (i.e. the index is not valid).
    bool isIndexVisible(const QModelIndex & index) const;

    /// @brief Resolves the provided URI from the current index path.

    /// The current index must be a valid index.
    /// @param uri The URI to resolve. Must of scheme @c URI::Scheme::CPATH.
    /// @return Returns the complete path.
    common::URI completeRelativePath(const common::URI & uri) const;

    /// @name Signals
    /// @{

    /// @brief Signal called when the tree needs to be updated

    /// @param node New tree
    void list_tree_reply(cf3::common::SignalArgs & node);

    /// @} END Signals

    void contentListed(Component::Ptr node);

    static Ptr globalTree();

  public slots:

    /// @brief Clears the tree.
    /// Deletes all component that are not client component.
    void clearTree();

    /// @brief Sends a request to update de tree
    void updateTree();

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

    void beginUpdateTree();

    void endUpdateTree();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

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

    /// @brief Mutex to control concurrent access.
    QMutex * m_mutex;

    /// @brief Converts an index to a tree node

    /// @param index Node index to convert
    /// @return Returns the tree node, or @c nullptr if the index could
    /// not be converted (i.e. index is invalid)
    TreeNode * indexToTreeNode(const QModelIndex & index) const;

    /// @brief Converts an index to a node

    /// @param index Node index to convert
    /// @return Returns the node, or a null shared pointer if the index could
    /// not be converted (i.e. index is invalid)
    CNode::Ptr indexToNode(const QModelIndex & index) const;

    /// @brief Retrieves a node path from its index.

    /// This is a recursive method.
    /// @param index Node index.
    /// @param path Intermediate retrieved path
    void buildNodePathRec(const QModelIndex & index, QString & path) const;

    /// @brief Recursively checks whether a node name or one of its children
    /// matches a provided regular expression.
    /// The check stops once a recursive call returns @c true
    /// @param node The node to check.
    /// @param regex Regular expression to match.
    /// @return Returns @c true if the node or at least one of its children
    /// matches the regular expression.
    bool nodeMatchesRec(Component::Ptr node, const QRegExp regex) const;

  }; // class NTree

  ///////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Core_NTree_hpp
