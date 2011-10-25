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
    void set_tree_root(NRoot::Ptr node);

    /// @brief Gives the current root
    /// @return Returns the current root
    NRoot::Ptr tree_root() const;

    /// @brief Sets the current index.

    /// If @c newIndex is valid and different from the current index, the
    /// current index is changed and #current_index_changed signal is emitted.
    /// If @c newIndex is either not valid or the same as the current index,
    /// nothing is done.
    /// @param newIndex The new index.
    /// @see getCurrentIndex
    /// @see current_index_changed
    void set_current_index(const QModelIndex & newIndex);

    /// @brief Gives the current index

    /// The current index may not be valid if @c #changeCurrentIndex was
    /// never called.
    /// @return Returns the current index.
    /// @see setCurrentIndex.
    QModelIndex current_index() const;

    /// @brief Gives the path of the current index.
    /// @return Returns the path of the index returned by @c #getCurrentIndex()
    /// or an empty path if not valid current index is set.
    cf3::common::URI current_path() const;

    /// @brief Gets node options

    /// @param index Node index
    /// @param options List where options will be stored
    /// @param ok If not @c nullptr, used to strore whether the option
    /// gathering succeded or not.
    void list_node_options(const QModelIndex & index,
                         QList<cf3::common::Option::ConstPtr> & options,
                         bool * ok = nullptr) const;

    /// @brief Gets node properties

    /// @param index Node index
    /// @param props Map where properties will be stored. The key is the
    /// property name, the value is the property value.
    /// @param ok If not @c nullptr, used to store whether the property
    /// gathering succeeded or not.
    void list_node_properties(const QModelIndex & index,
                            QMap<QString, QString> & params,
                            bool * ok = nullptr) const;

    /// @brief Gets node actions

    /// @param index Node index
    /// @param actions List where action will be stored.
    /// @param ok If not @c nullptr, used to store whether the action
    /// gathering succeeded or not (i.e. it fails if the index is not valid).
    void list_node_actions(const QModelIndex & index, QList<ActionInfo> & actions,
                         bool * ok = nullptr) const;

    /// @brief Retrieves a node path.

    /// @param index Node index
    /// @return Returns the node path
    QString node_path(const QModelIndex & index) const;

    /// @brief Set advanced mode

    /// @param advancedMode If @c true, advanced mode is activated.
    void set_advanced_mode(bool advanceMode);

    /// @brief Indicates whether advanced mode is activated or not.

    /// @return Returns @c true if advanced mode is activated, otherwise,
    /// returns @c false.
    bool is_advanced_mode() const;

    /// @brief Checks whether two indexes point to the same node.

    /// If indexes point to a null node, they are considered as not
    /// pointing to the same node.
    /// @param left Left node
    /// @param right Right node
    /// @return Returns @c true if both indexes point to the same node.
    /// Otherwise returns @c false.
    bool are_from_same_node(const QModelIndex & left, const QModelIndex & right) const;

    /// @brief Retrieves a node from its path.

    /// @param path The node path
    /// @return Returns the found node, or a null shared pointer if
    /// the node does not exist.
    CNode::ConstPtr node_by_path(const cf3::common::URI & path) const;

    /// @brief Retrieves a node from its path.

    /// @param path The node path
    /// @return Returns the found node, or a null shared pointer if
    /// the node does not exist.
    CNode::Ptr node_by_path(const cf3::common::URI & path);

    /// @brief Retrieves a node index from its path.

    /// @param path The node index path
    /// @return Returns the found node index, or a invalid index if
    /// it does not exist.
    QModelIndex index_from_path(const cf3::common::URI & path) const;

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
    void modify_options(const QModelIndex & index,
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
    virtual QString tool_tip() const;

    //@} END VIRTUAL FUNCTIONS

    /// @brief Set the debug mode

    /// In debug mode, client components are showed.
    /// @param debugMode If @c true, the debug mode is activated. Otherwise,
    /// it is deactivated.
    void set_debug_mode_enabled(bool debugMode);

    /// @brief Indicates whether the debug mode is activated or not.

    /// @return Returns @c true if the debug mode is activated; otherwise,
    /// returns @c false.
    bool is_debug_mode_enabled() const;

    /// @brief Updates the children row counts, starting from the root.
    /// @note This method emits a @c layoutChanged() signal, causing the
    /// view(s) to be completely updated. Calling this method too often might
    /// decrease the program performances.
    void update_root_children();

    void options_changed(const cf3::common::URI & path);

    /// @brief Checks whether a node name or one of its children matches a
    /// provided regular expression.
    /// @param index Index of the node to check.
    /// @param regex Regular expression to match.
    /// @return Returns @c true if the node or at least one of its children
    /// matches the regular expression.
    bool node_matches(const QModelIndex & index, const QRegExp & regex) const;

    /// @brief Checks whether a nodeis visible or not.

    /// A node is visible if:
    /// @li it is basic
    /// @li it is advanced @b and advanced mode is activated
    /// @li it is a client component @b and debug mode is activated.
    /// @param index Index of the node to check
    /// @return Returns @c true if the node is visible. Otherwise, returns
    /// @c false (i.e. the index is not valid).
    bool check_index_visible(const QModelIndex & index) const;

    /// @brief Resolves the provided URI from the current index path.

    /// The current index must be a valid index.
    /// @param uri The URI to resolve. Must of scheme @c URI::Scheme::CPATH.
    /// @return Returns the complete path.
    common::URI complete_relativepath(const common::URI & uri) const;

    /// @name Signals
    /// @{

    /// @brief Signal called when the tree needs to be updated

    /// @param node New tree
    void list_tree_reply(cf3::common::SignalArgs & node);

    /// @} END Signals

    void content_listed(Component::Ptr node);

    static Ptr global();

  public slots:

    /// @brief Clears the tree.
    /// Deletes all component that are not client component.
    void clear_tree();

    /// @brief Sends a request to update de tree
    void update_tree();

  signals:

    /// @brief Signal emitted when the current index has changed.

    /// @param newIndex The new current index
    /// @param oldIndex The old current index
    /// @see setCurrentIndex
    void current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    /// @brief Signal emitted when advanced mode changed

    /// @param advanced New advanced mode status
    /// @see setAdvancedMode
    void advanced_mode_changed(bool advanced);

    void begin_update_tree();

    void end_update_tree();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}

  private:

    /// @brief Column titles
    QStringList m_columns;

    /// @brief The root node
    TreeNode * m_root_node;

    /// @brief Current index
    QModelIndex m_current_index;

    /// @brief Indicates whether we are in advanced mode or not
    bool m_advanced_mode;

    /// @brief Indicates whether we are in debug mode or not
    bool m_debug_mode_enabled;

    /// @brief Mutex to control concurrent access.
    QMutex * m_mutex;

    /// @brief Converts an index to a tree node

    /// @param index Node index to convert
    /// @return Returns the tree node, or @c nullptr if the index could
    /// not be converted (i.e. index is invalid)
    TreeNode * index_to_tree_node(const QModelIndex & index) const;

    /// @brief Converts an index to a node

    /// @param index Node index to convert
    /// @return Returns the node, or a null shared pointer if the index could
    /// not be converted (i.e. index is invalid)
    CNode::Ptr index_to_node(const QModelIndex & index) const;

    /// @brief Retrieves a node path from its index.

    /// This is a recursive method.
    /// @param index Node index.
    /// @param path Intermediate retrieved path
    void build_node_path_recursive(const QModelIndex & index, QString & path) const;

    /// @brief Recursively checks whether a node name or one of its children
    /// matches a provided regular expression.
    /// The check stops once a recursive call returns @c true
    /// @param node The node to check.
    /// @param regex Regular expression to match.
    /// @return Returns @c true if the node or at least one of its children
    /// matches the regular expression.
    bool node_matches_recursive(Component::Ptr node, const QRegExp regex) const;

  }; // class NTree

  ///////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Core_NTree_hpp
