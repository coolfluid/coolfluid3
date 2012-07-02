// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_FileFilter_hpp
#define cf3_ui_Graphics_FileFilter_hpp

#include <QSortFilterProxyModel>

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace core { class NRemoteFSBrowser; }

namespace graphics {

//////////////////////////////////////////////////////////////////////////////

class FileFilter : public QSortFilterProxyModel
{
  Q_OBJECT

public:

  FileFilter( core::NRemoteFSBrowser * model, QObject * parent = 0 );

  virtual QVariant data ( const QModelIndex &index, int role ) const;

protected:

  QMimeData * mimeData(const QModelIndexList &indexes) const;

private: // data

  core::NRemoteFSBrowser * m_model;
};

//////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_FileFilter_hpp
