// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Graphics_FileFilter_hpp
#define CF_UI_Graphics_FileFilter_hpp

#include <QSortFilterProxyModel>

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

namespace Core { class NRemoteFSBrowser; }

namespace Graphics {

//////////////////////////////////////////////////////////////////////////////

class FileFilter : public QSortFilterProxyModel
{
  Q_OBJECT

public:

  FileFilter( Core::NRemoteFSBrowser * model, QObject * parent = 0 );

  virtual QVariant data ( const QModelIndex &index, int role ) const;

private: // data

  Core::NRemoteFSBrowser * m_model;
};

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Graphics_FileFilter_hpp
