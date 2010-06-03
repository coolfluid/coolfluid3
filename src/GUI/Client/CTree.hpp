#ifndef CF_GUI_Client_CTree_hpp
#define CF_GUI_Client_CTree_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>

#include "GUI/Client/CNode.hpp"

namespace CF {
namespace GUI {
namespace Client {

  ///////////////////////////////////////////////////////////////////////////

  class CNode;
  class TreeNode;

  /// @brief Tree model

  class CTree : public QAbstractItemModel
  {
  public:

    CTree(CNode::Ptr rootNode);

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
  private:

    TreeNode * m_rootNode;


  }; // class CTree

  ///////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CTree_hpp
