#ifndef CF_GUI_Client_CTree_hpp
#define CF_GUI_Client_CTree_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QStringList> /// @todo does not compile without that...but why ???

#include "Common/Component.hpp"
#include "GUI/Client/TreeNode.hpp"
#include "GUI/Client/CNode.hpp"


class QDomElement;
class QModelIndex;

namespace CF {

namespace Common { class CPath; }

namespace GUI {
namespace Client {

  ///////////////////////////////////////////////////////////////////////////

  class TreeNode;

  /// @brief Tree model

  class CTree :
      public QAbstractItemModel,
      public CF::Common::Component
  {
    Q_OBJECT
  public:

    typedef boost::shared_ptr<CTree> Ptr;
    typedef boost::shared_ptr<CTree const> ConstPtr;

    /// @brief Constructor.

    /// @param rootNode The root node. May be @c CFNULL.
    CTree(CF::GUI::Client::CNode::Ptr rootNode = CF::GUI::Client::CNode::Ptr());

    /// @brief Replaces the current component tree.

    /// The old tree is destroyed (regarding to @c boost::shared_ptr delete
    /// rules).
    /// @param node The new root. May be @c CFNULL.
    void setRoot(CNode::Ptr node);

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

    void getNodeParams(const QModelIndex & index,
                       QList<NodeOption> & params, bool * ok = CFNULL) const;

    QString getNodePath(const QModelIndex & index) const;

    void setAdvancedMode(bool advanceMode);

    bool isAdvancedMode() const;

    bool areFromSameNode(const QModelIndex & left, const QModelIndex & right) const;

    bool haveSameData(const QModelIndex & left, const QModelIndex & right) const;

    CNode::Ptr getNodeByPath(const CF::Common::CPath & path) const;

    QModelIndex getIndexByPath(const CF::Common::CPath & path) const;

    QModelIndex nodeToIndex(const CNode::Ptr & node) const;

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

    void showNodeMenu(const QModelIndex & index, const QPoint & pos) const;

  signals:

    /// @brief Signal emitted when the current index has changed.

    /// @param newIndex The new current index
    void currentIndexChanged(const QModelIndex & newIndex, const QModelIndex & oldIndex);

    void advancedModeChanged(bool advanced);

  private:

    QStringList m_columns;

    TreeNode * m_rootItem;

    QModelIndex m_currentIndex;

    bool m_advancedMode;

    inline TreeNode * indexToTreeNode(const QModelIndex & index) const
    {
      return static_cast<TreeNode *>(index.internalPointer());
    }

    inline CNode::Ptr indexToNode(const QModelIndex & index) const
    {
      return this->indexToTreeNode(index)->getNode();
    }

    void getNodePathRec(const QModelIndex & index, QString & path) const;

  private: // boost signals

    /// @name Signals
    /// @{

    CF::Common::Signal::return_t updateTree(CF::Common::Signal::arg_t & node);

    /// @} END Signals


  }; // class CTree

  ///////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CTree_hpp
