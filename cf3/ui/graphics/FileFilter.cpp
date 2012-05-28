// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>

#include "ui/core/NRemoteFSBrowser.hpp"

#include "ui/graphics/FileFilter.hpp"

#include <QMimeData>

using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////////

FileFilter::FileFilter( NRemoteFSBrowser * model, QObject * parent )
  : QSortFilterProxyModel( parent ),
    m_model( model )
{
  cf3_assert( is_not_null(model) );

  setSourceModel(m_model);
}

//////////////////////////////////////////////////////////////////////////////

QVariant FileFilter::data ( const QModelIndex &index, int role ) const
{
  QVariant value;
  static QFileIconProvider provider; // heavy to initialize, better to put it as static

  if(index.isValid())
  {
    if( role == Qt::DecorationRole && index.column() == 0 )
    {
      QModelIndex indexInModel = mapToSource(index);

      if( m_model->is_directory(indexInModel) )
        value = provider.icon( QFileIconProvider::Folder );
      else if( m_model->is_file(indexInModel) )
        value = provider.icon( QFileIconProvider::File );
    }
    else
      value = QSortFilterProxyModel::data(index, role);
  }

  return value;
}

//////////////////////////////////////////////////////////////////////////////

QMimeData * FileFilter::mimeData(const QModelIndexList &indexes) const
{
  //we don't really care about the data itself, all matter is that this pointer is not null
  return new QMimeData();
}

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

