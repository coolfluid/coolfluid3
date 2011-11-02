// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_PropertyModel_hpp
#define cf3_ui_core_PropertyModel_hpp

////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>
#include <QStringList>

#include "ui/core/LibCore.hpp"

////////////////////////////////////////////////////////////////////////////

class QModelIndex;
class QString;

template<class T> class QList;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

  struct Core_API PropertyItem
  {
    QString m_name;

    QString m_value;

    unsigned int m_row;

    PropertyItem(const QString & name, const QString & value, unsigned int row)
      : m_name(name), m_value(value), m_row(row) {}

  }; // struct PropertyItem

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Model that maintains properties for the node pointed
  /// by the index returned by <code>NTree::global()->currentIndex()</code>.
  /// This class is a view for the tree model and is automatically
  /// updated whenever the current index is changed.
  /// @author Quentin Gasper.

  class Core_API PropertyModel : public QAbstractItemModel
  {
    Q_OBJECT

  public:

    /// @brief Constructor
    PropertyModel();

    /// @brief Destructor.

    /// Frees all allocated memory.
//    ~PropertyModel();

    /// @brief Implementation of @c QAbstractItemModel::data().

    /// Only the role @c Qt::DisplayRole is accepted.
    /// Other roles will result to the return of an empty @c QVariant object
    /// (built with the default construtor).
    /// @param index Concerned item index.
    /// @param role Role of the returned value (only @c Qt::DisplayRole).
    /// @return Returns an empty QVariant object if the role is not
    /// @c Qt::DecorationRole or if the index is not valid. Otherwise,
    /// returns the nodename of the item at the specified index.
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

  private slots:

    /// @brief Slot called when the current index has changed in the
    /// tree model.

    /// @param newIndex New current index.
    /// @param oldIndex Old current index.
    void current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex);

  private:

    /// @brief Model data.
    QList<PropertyItem *> m_data;

    /// @brief List of column headers
    QStringList m_columns;

    /// @brief Clean the model data.
    void empty_list();

  }; // class PropertyModel

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_PropertyModel_hpp
