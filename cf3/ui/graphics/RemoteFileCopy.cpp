// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/graphics/RemoteFileCopy.hpp"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QApplication>
#include "ui/core/ThreadManager.hpp"
#include "ui/core/NBrowser.hpp"
#include "ui/core/TreeThread.hpp"
#include "ui/graphics/FileFilter.hpp"
#include "ui/core/SSHTunnel.hpp"
#include "ui/core/NRemoteFSBrowser.hpp"
#include "common/XML/SignalFrame.hpp"
#include "ui/core/NetworkQueue.hpp"
#include <QFileSystemModel>
#include "ui/uicommon/ComponentNames.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

RemoteFileCopy::RemoteFileCopy(QWidget* parent) : QSplitter(parent) {
  local_files=new QFileSystemModel(this);
  remote_files=boost::shared_ptr<core::NRemoteFSBrowser>(
        new core::NRemoteFSBrowser(core::NBrowser::global()->generate_name()));
  filter_remote_model=new FileFilter(remote_files.get(),this);
  core::ThreadManager::instance().tree().root()->add_node(remote_files);
  local_list_widget=new DraggableListWidget(this);
  remote_list_widget=new DraggableListWidget(this);
  QGroupBox* local_box=new QGroupBox("Local File System",this);
  QGroupBox* remote_box=new QGroupBox("Remote File System",this);
  QVBoxLayout* local_layout=new QVBoxLayout(this);
  QVBoxLayout* remote_layout=new QVBoxLayout(this);
  local_layout->addWidget(local_list_widget);
  remote_layout->addWidget(remote_list_widget);
  local_box->setLayout(local_layout);
  remote_box->setLayout(remote_layout);
  addWidget(local_box);
  addWidget(remote_box);

  filter_remote_model->setDynamicSortFilter(true);
  filter_remote_model->setFilterRole(Qt::DisplayRole);
  filter_remote_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  local_list_widget->set_accepted_source(remote_list_widget);
  remote_list_widget->set_accepted_source(local_list_widget);
  local_list_widget->setModel(local_files);
  remote_list_widget->setModel(filter_remote_model);
  local_files->setRootPath("/");
  local_files->setFilter(QDir::AllEntries);
  local_list_widget->setRootIndex(local_files->index(QApplication::applicationDirPath()));
  remote_files->open_dir("");

  connect(local_list_widget,SIGNAL(copy_request(QModelIndex,QModelIndexList))
          ,this,SLOT(to_local_copy_requested(QModelIndex,QModelIndexList)));
  connect(remote_list_widget,SIGNAL(copy_request(QModelIndex,QModelIndexList))
          ,this,SLOT(to_remote_copy_requested(QModelIndex,QModelIndexList)));
  connect(local_list_widget,SIGNAL(item_double_clicked(QModelIndex))
          ,this,SLOT(local_item_double_clicked(QModelIndex)));
  connect(remote_list_widget,SIGNAL(item_double_clicked(QModelIndex))
          ,this,SLOT(remote_item_double_clicked(QModelIndex)));
  connect(remote_files.get(),SIGNAL(current_path_changed(const QString &))
          ,this,SLOT(remote_dir_changed(const QString &)));
  connect(remote_files.get(),SIGNAL(copy_finished),this,SLOT(copy_finished));
}

////////////////////////////////////////////////////////////////////////////////

RemoteFileCopy::~RemoteFileCopy(){
  //core::ThreadManager::instance().tree().root()->remove_node(remote_files->name());
  //delete remote_files;
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::to_local_copy_requested(QModelIndex item, QModelIndexList copy_items){
  std::vector<std::string> params;
  std::string local_signature=core::SSHTunnel::get_local_signature();
  local_signature.append(":");
  if (item.isValid()){
    if (local_files->isDir(item))
      local_signature.append(local_files->filePath(item).toStdString());
    else
      local_signature.append(local_files->filePath(local_files->parent(item)).toStdString());
  }else{
    item=local_list_widget->rootIndex();
  }
  foreach (const QModelIndex & to_copy, copy_items){
    QModelIndex to_copy_not_const = filter_remote_model->mapToSource(to_copy);
    if (remote_files->is_directory(to_copy_not_const))
      params.push_back("-r");
    params.push_back("-p");
    params.push_back(remote_files->retrieve_full_path(to_copy_not_const).toStdString());
    params.push_back(local_signature);
    params.push_back("#");
  }
  remote_files->copy_request(params);
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::to_remote_copy_requested(QModelIndex item, QModelIndexList copy_items){
  std::string dest;
  std::vector<std::string> params;
  std::string local_signature=core::SSHTunnel::get_local_signature().append(":");
  item = filter_remote_model->mapToSource(item);
  if (item.isValid() && remote_files->is_directory(item))
    dest=remote_files->retrieve_full_path(item).toStdString();
  else
    dest=remote_files->current_path().toStdString();
  foreach (const QModelIndex & to_copy, copy_items){
    if (local_files->isDir(to_copy))
      params.push_back("-r");
    params.push_back("-p");
    params.push_back(std::string(local_signature).append(local_files->filePath(to_copy).toStdString()));
    params.push_back(dest);
    params.push_back("#");
  }
  remote_files->copy_request(params);
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::local_item_double_clicked(QModelIndex item){
  if (local_files->isDir(item)){
    item=local_files->index(local_files->filePath(item));
    if (item.column() >= 0)
      local_list_widget->setRootIndex(item);
  }
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::remote_item_double_clicked(QModelIndex item){
  item = filter_remote_model->mapToSource(item);
  if (remote_files->is_directory(item))
    remote_files->open_dir(remote_files->retrieve_full_path(item));
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::remote_dir_changed(const QString & dir){
  static QRegExp dotdot("/[^/]*/\\.\\./");
  QString without_dot=QString(dir).replace("/./","/");
  without_dot.replace(dotdot,"/");
  if (dir != without_dot)
    remote_files->open_dir(without_dot);
}

////////////////////////////////////////////////////////////////////////////////

void RemoteFileCopy::copy_finished(){
  qDebug() << "replied received" ;
  remote_files->open_dir(remote_files->current_path());
  //local_files->
}

////////////////////////////////////////////////////////////////////////////////

DraggableListWidget::DraggableListWidget(QWidget* parent)
  : QListView(parent){
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setDragEnabled(true);
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDefaultDropAction(Qt::CopyAction);
  setAlternatingRowColors(true);
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::set_accepted_source(DraggableListWidget* accepted_source){
  this->accepted_source=accepted_source;
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::mouseDoubleClickEvent(QMouseEvent *e){
  emit item_double_clicked(indexAt(e->pos()));
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::dropEvent(QDropEvent *e){
  //qDebug() << "dropEvent" ;
  if (e->source() == accepted_source){
    emit copy_request(indexAt(e->pos()),accepted_source->selectedIndexes());
  }
  e->ignore();
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::dragEnterEvent(QDragEnterEvent *e){
  //qDebug() << "dragEnterEvent" ;
  e->acceptProposedAction();
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::dragMoveEvent(QDragMoveEvent *e){
  e->acceptProposedAction();
}

////////////////////////////////////////////////////////////////////////////////

void DraggableListWidget::dragLeaveEvent(QDragLeaveEvent *e){
  //qDebug() << "dragLeaveEvent" ;
  e->accept();
}

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////
