// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>

#include "UI/Core/NRemoteFSBrowser.hpp"

#include "UI/Graphics/FileFilter.hpp"

using namespace cf3::UI::Core;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

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
    if( role == Qt::DecorationRole )
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

} // Graphics
} // UI
} // cf3

