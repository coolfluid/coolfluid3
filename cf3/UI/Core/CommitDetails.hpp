// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_CommitDetails_h
#define cf3_GUI_Core_CommitDetails_h

////////////////////////////////////////////////////////////////////////////////

#include <QAbstractItemModel>

#include "UI/Core/LibCore.hpp"

class QStringList;
class QString;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

  class CommitDetailsItem;

  /// @brief Model that handles modified options details.

  /// It can be used in a view to show modified options, their old and
  /// current value. When a method takes as parameter a @c QModelIndex
  /// that can be null, the root is taken is this case.
  /// @author Quentin Gasper.

  class Core_API CommitDetails : public QAbstractItemModel
  {

  public:

    /// @brief Constructor.

    /// @param parent The parent object. May be @c nullptr.
    /// @param nodePath Path to the node the options belong to. May be empty.
    CommitDetails(QObject * parent = nullptr, const QString & nodePath = QString());

    /// @brief Implements QAbstractItemModel::data()

    /// Gives index data, depending on the provided role.
    /// @param index The index from which the data is wanted
    /// @param role Role of the wanted data. Available roles are defined
    /// by <code>Qt::ItemDataRole</code> enum. <code>Qt::DisplayRole</code>
    /// is the only role accepted.
    /// @param Returns the wanted data, of an invalid @c QVariant (built with
    /// default constructor) if the index is not valid, or if the role
    /// is not supported.
    QVariant data(const QModelIndex &index, int role) const;

    /// @brief Reimplements QAbstractItemModel::headerData()

    /// Gives header data for a specified column or row.
    /// @param section Column or row number
    /// @param orientation Indicates wether we want the row header or
    /// the column one.
    /// @param role Role of the wanted data. Available roles are defined
    /// by <code>Qt::ItemDataRole</code> enum. <code>Qt::DisplayRole</code>
    /// is the only role accepted.
    /// @return Returns:
    /// @li the column name if @c orientation is <code>Qt::Horizontal</code>
    /// @li "Option #i", where 'i' is <code>section + 1</code>, if
    /// @c orientation is <code>Qt::Vertical</code>
    /// @li an invalid @c QVariant (built with default constructor) if
    /// section is less than 0 or bigger than the number of columns or rows.
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /// @brief Implements QAbstractItemModel::index()

    /// Builds an index for the wanted object
    /// @param row Row
    /// @param column Column
    /// @param parent Parent index, may be null.
    /// @return Returns the built index, or a invalid index if either the
    /// row or the column is not valid.
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;

    /// @brief Implements QAbstractItemModel::parent()

    /// Gives the parent index of a provided index.
    /// @brief index Index from which we want to know the parent index.
    /// @note Since this is not a tree-based model, this implementation
    /// always returns a invalid index.
    /// @return Returns the parent index.
    QModelIndex parent(const QModelIndex &index) const;

    /// @brief Implements QAbstractItemModel::rowCount()

    /// Gives the number of children under a parent
    /// @param parent The parent. May be null.
    /// @return Returns the number of children under @c parent.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /// @brief Implements QAbstractItemModel::columnCount()

    /// Gives the number of columns for a specified index.
    /// @param parent The index of which we want to know the column count.
    /// @return Always returns 3.
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /// @brief Sets a option.

    /// If the option already exists, ites are modified. Otherwise, the
    /// option is created.
    /// @param optionName Option name
    /// @param oldValue Old Value. May be empty.
    /// @param currentValue Current value. May be empty.
    void setOption(const QString & optionName, const QString & oldValue,
                   const QString & currentValue);

    /// @brief Gives the node path
    /// @return Returns the node path. This string may be empty if it
    /// has never been set.
    QString nodePath() const;

    /// @brief Sets the node path
    /// @param nodePath Node path.
    void setNodePath(const QString & nodePath);

    /// @brief Checks whether the internal container has options.
    /// @return Returns @c true if there is at least one option;
    /// otherwise returns @c false.
    bool hasOptions() const;

    /// @brief Removes all options and clears the node path
    /// @see clearOptions
    void clear();

  private:

    /// @brief Node path
    QString m_nodePath;

    /// @brief Model items.
    QList<CommitDetailsItem *> m_items;

  }; // class CommitDetails

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

#endif // cf3_GUI_Core_CommitDetails_h
