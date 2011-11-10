// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_graphics_FavoritesModel_hpp
#define cf3_ui_graphics_FavoritesModel_hpp

#include <QAbstractItemModel>
#include <QFileIconProvider>

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////////

/// Model that manages a list of favorite directories.

/// The model has two type of favorite directies : the normal favorites and
/// the special ones. The normal favorites are managed by the server that saves
/// the list to the disc and sends it to the GUI. It contains absolute paths to
/// the directories. The special favorites are keywords giving access to special
/// directories on the the server side, such as the remote home directory. @n

/// The model accepts new entries and entries removing with the following
/// conditions:
/// @li An entry can only be added once. In other words, to favorites cannot
/// point to the same path.
/// @li An entry that describes a special favorite cannot be removed.

/// Whenever a favorite is added or removed, the new list is directly sent to
/// the server.

/// @author Quentin Gasper.
class FavoritesModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  /// Constructor.
  FavoritesModel();

  /// Implements @c QAbstractItemModel::data.

  /// @param index Index of the wanted data.
  /// @param role Desired role. Only @c Qt::DisplayRole (text),
  /// @c Qt::DecorationRole (icon) and @c Qt::ToolTipRole roles are accepted.
  /// @return Returns the desired value in the form of a @c QVariant. If the
  /// @c index or the @c role is not valid, an invalid @c QVariant is returned.
  /// In the case of @c Qt::DisplayRole, only the directory name is returned
  /// (not the full path).
  virtual QVariant data ( const QModelIndex & index, int role ) const;

  /// Implements @c QAbstractItemModel::parent.

  /// Gives the parent index of a specified index.
  /// @param child The child index.
  /// @return Returns the parent index, or an invalid one if the specified index
  /// is not valid or has no parent.
  virtual QModelIndex parent ( const QModelIndex & child ) const;

  /// Implements @c QAbstractItemModel::index.

  /// Gives a child index of a specified parent, at a specified position.
  /// @param row The row number of the wanted index.
  /// @param column The column number of te wanted index.
  /// @param parent The parent index.
  /// @return Returns the index, or an invalid index if it does not exist.
  virtual QModelIndex index ( int row, int column, const QModelIndex & parent ) const;

  /// Implements @c QAbstractItemModel::rowCount.

  /// Gives the row count (number of children) for a specified parent.
  /// @param parent The parent.
  /// @return Returns number of children the parent has.
  virtual int rowCount ( const QModelIndex &parent ) const;

  /// Implements @c QAbstractItemModel::columnCount.

  /// Gives the column count for a specified index.
  /// @param parent The index.
  /// @return Always returns 1.
  virtual int columnCount ( const QModelIndex &parent ) const;

  /// Gives the list of favorites.

  /// The special places are not present in this list.
  /// @return Returns the list of favorites.
  QStringList string_list () const;

  /// Adds a string.

  /// @param str The string to add.
  /// @return Returns @c true if the string was successfuly added or @c false
  /// if the string was already present in the list.
  bool add_string( const QString & str );

  /// Removes a string.

  /// @param i Index of the string to remove
  /// @return Returns @c true if the string was removed successfully or @c false
  /// if the index was not out of the list bounds or is the one of a special
  /// favorite.
  bool remove_string( int i );

  /// Indicates whether the favorite directory pointed by the index is a special
  /// directory.

  /// @param index The index.
  /// @return Returns @c true if the directory is a special one; otherwose,
  /// returns @c false.
  bool is_special_dir( const QModelIndex & index ) const;

  /// Gives the full path of teh directory pointed by the provided index.

  /// @param index The index.
  /// @return Returns the directory full path, the favority directory name or
  /// an empty string if the index if not valid.
  QString full_path ( const QModelIndex & index ) const;

public slots:

  /// Sets the string list.
  /// @param list The list to set.
  void set_string_list( const QStringList & list );

private: // nested struct

  /// Internal structure to manage special directories data.
  /// @author Quentin Gasper.
  struct SpecialDir
  {
    /// The directory name.
    QString name;

    /// The directory description.
    QString description;

    /// The directory icon.
    QIcon icon;

    /// Convenience constructor.

    /// @param name Directory name.
    /// @param descr Directory description.
    /// @param icon Directory icon.
    SpecialDir( const QString & name, const QString & descr, const QIcon & icon );
  };

private: // data

  /// List of special directory
  QList<SpecialDir> m_special_dirs;

  /// Model data.
  QStringList m_data;

  /// The icon provider.

  /// This class is heavy to initiate, so we create one instance for the object
  /// lifetime, instead of creating a new one each time the view calls data().
  QFileIconProvider m_provider;

}; // FavoritesModel

//////////////////////////////////////////////////////////////////////////////

} // graphics
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_FavoritesModel_hpp
