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

/// @author Quentin Gasper.
class FavoritesModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  /// Constructor.
  FavoritesModel();

  /// Reimplements @c QStringListModel::data.

  /// @param index Index of the wanted data.
  /// @param role Desired role. Only @c Qt::DisplayRole (text) is accepted.
  /// @return Returns the desired value in the form of a @c QVariant. If the
  /// @c index or the @c role is not valid, an invalid @c QVariant is returned.
  virtual QVariant data ( const QModelIndex & index, int role ) const;

  /// Reimplements @c QStringListModel::parent.

  /// Gives the parent index of a specified index.
  /// @param child The child index.
  /// @return Returns the parent index, or an invalid one if the specified index
  /// is not valid or has no parent.
  virtual QModelIndex parent ( const QModelIndex & child ) const;

  /// Gives an index of a specified parent, at a specified position.

  virtual QModelIndex index ( int row, int column, const QModelIndex & parent ) const;

  virtual int rowCount ( const QModelIndex &parent ) const;

  virtual int columnCount ( const QModelIndex &parent ) const;

  QStringList string_list () const;

  bool add_string( const QString & str );

  bool remove_string( int i );

  bool is_special_dir( const QModelIndex & index ) const;

  QString full_path ( const QModelIndex & index ) const;

public slots:

  void set_string_list( const QStringList & list );

private: // nested struct

  struct SpecialDir
  {
    QString name;

    QString description;

    QIcon icon;

    SpecialDir( const QString & name, const QString & descr, const QIcon & icon );
  };

private: // data

  QList<SpecialDir> m_special_dirs;

  QStringList m_data;

  QFileIconProvider m_provider;

}; // FavoritesModel

//////////////////////////////////////////////////////////////////////////////

} // graphics
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_FavoritesModel_hpp
