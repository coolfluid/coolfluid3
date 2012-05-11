// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_graphics_RemoteFileCopy_hpp
#define cf3_ui_graphics_RemoteFileCopy_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QList>
#include "ui/graphics/LibGraphics.hpp"
#include <QListView>
#include <QSplitter>
#include <QDropEvent>
#include <boost/shared_ptr.hpp>
#include <QDebug>

class QFileSystemModel;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace core { class NRemoteFSBrowser; }

namespace graphics {

class DraggableListWidget;
class FileFilter;

class Graphics_API RemoteFileCopy : public QSplitter{
  Q_OBJECT
public:
  RemoteFileCopy(QWidget *parent);
  ~RemoteFileCopy();
private slots:
  void to_local_copy_requested(QModelIndex item, QModelIndexList copy_items);
  void to_remote_copy_requested(QModelIndex item, QModelIndexList copy_items);
  void local_item_double_clicked(QModelIndex item);
  void remote_item_double_clicked(QModelIndex item);
  void remote_dir_changed(const QString & dir);
  void copy_finished();
private:
  boost::shared_ptr<core::NRemoteFSBrowser> remote_files;
  FileFilter* filter_remote_model;//for icons
  QFileSystemModel* local_files;
  DraggableListWidget* remote_list_widget;
  DraggableListWidget* local_list_widget;
};

////////////////////////////////////////////////////////////////////////////////

class DraggableListWidget : public QListView {
  Q_OBJECT
public:
  /// @brief constructor
  DraggableListWidget(QWidget* parent);
  void set_accepted_source(DraggableListWidget* accepted_source);
signals:
  void copy_request(QModelIndex item, QModelIndexList copy_items);
  void item_double_clicked(QModelIndex item);
protected:
  void mouseDoubleClickEvent(QMouseEvent *e);
  void dropEvent(QDropEvent *e);
  void dragEnterEvent(QDragEnterEvent *e);
  void dragMoveEvent(QDragMoveEvent *e);
  void dragLeaveEvent(QDragLeaveEvent *e);
private:
  DraggableListWidget* accepted_source;
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_RemoteFileCopy_hpp
