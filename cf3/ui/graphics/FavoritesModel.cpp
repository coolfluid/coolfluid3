// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDir>
#include <QFileIconProvider>

#include "ui/graphics/FavoritesModel.hpp"

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////////

FavoritesModel::SpecialDir::SpecialDir( const QString & name,
                                        const QString & descr,
                                        const QIcon & icon )
  : name(name),
    description(descr),
    icon(icon)
{
}

//////////////////////////////////////////////////////////////////////////////

FavoritesModel::FavoritesModel()
{
  m_special_dirs.append( SpecialDir("Home",
                                    "Opens your remote home directory.",
                                    m_provider.icon(QFileInfo(QDir::homePath()))) );

  set_string_list( QStringList() );
}

//////////////////////////////////////////////////////////////////////////////

QVariant FavoritesModel::data ( const QModelIndex & index, int role ) const
{
  QVariant return_value;
  int row = index.row();

  if( index.isValid() && index.column() == 0 && row >= 0 )
  {
    bool is_special = is_special_dir(index);

    if( role == Qt::DisplayRole )
      return_value = QFileInfo(m_data[row]).fileName();
    else if( role == Qt::DecorationRole )
      return_value = is_special ? m_special_dirs[row].icon : m_provider.icon(QFileIconProvider::Folder);
    else if( role == Qt::ToolTipRole )
      return_value = is_special ? m_special_dirs[row].description : m_data[row];
  }

  return return_value;
}

//////////////////////////////////////////////////////////////////////////////

QModelIndex FavoritesModel::parent ( const QModelIndex & child ) const
{
  return QModelIndex();
}

//////////////////////////////////////////////////////////////////////////////

QModelIndex FavoritesModel::index ( int row, int column, const QModelIndex & parent ) const
{
  QModelIndex index;

  if(this->hasIndex(row, column, parent))
  {
    if(!parent.isValid())
      index = createIndex(row, column, (void*) &m_data[row]);
  }

  return index;
}

//////////////////////////////////////////////////////////////////////////////

int FavoritesModel::rowCount ( const QModelIndex &parent ) const
{
  if( !parent.isValid() )
    return m_data.count();

  return 0;
}

//////////////////////////////////////////////////////////////////////////////

int FavoritesModel::columnCount ( const QModelIndex &parent ) const
{
  return 1;
}

//////////////////////////////////////////////////////////////////////////////

void FavoritesModel::set_string_list( const QStringList & list )
{
  QList<SpecialDir>::iterator it = m_special_dirs.begin();

  m_data.clear();

  for( ; it != m_special_dirs.end() ; ++it)
    m_data.append( it->name );

  m_data.append( list );

  reset();
}

//////////////////////////////////////////////////////////////////////////////

QStringList FavoritesModel::string_list () const
{
  QStringList list;

  for(int i = m_special_dirs.count() ; i < m_data.count() ; ++i )
    list.append(m_data[i]);

  return list;
}

//////////////////////////////////////////////////////////////////////////////

bool FavoritesModel::add_string(const QString &str)
{
  if( m_data.contains(str) )
    return false;

  m_data.append(str);
  reset();
  return true;
}

//////////////////////////////////////////////////////////////////////////////

bool FavoritesModel::remove_string(int i)
{
  if( i < m_special_dirs.count() || i >= m_data.count() )
    return false;

  m_data.removeAt(i);
  reset();
  return true;
}

//////////////////////////////////////////////////////////////////////////////

bool FavoritesModel::is_special_dir(const QModelIndex & index) const
{
  // row() returns -1 if index is not valid
  return index.row() >= 0 && index.row() < m_special_dirs.count();
}

//////////////////////////////////////////////////////////////////////////////

QString FavoritesModel::full_path ( const QModelIndex & index ) const
{
  return index.isValid() ? m_data[index.row()] : QString();
}

//////////////////////////////////////////////////////////////////////////////

} // graphics
} // ui
} // cf3
