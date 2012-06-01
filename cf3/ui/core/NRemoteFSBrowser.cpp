// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"

#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "common/XML/SignalOptions.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NetworkQueue.hpp"

#include "ui/core/NRemoteFSBrowser.hpp"

#include "common/XML/FileOperations.hpp"


using namespace cf3::common;
using namespace cf3::common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::NRemoteFSBrowser( const std::string & name ) :
  CNode(name, "NRemoteFSBrowser", CNode::DEBUG_NODE),
  m_include_no_extensions(false),
  m_include_files(true)
{
  m_completion_model = new QStringListModel(this);

  m_headers << "Name" << "Size" << "Date Modified";

  regist_signal("read_dir")
      .connect(boost::bind(&NRemoteFSBrowser::reply_read_dir, this, _1));
  regist_signal("list_favorites")
      .connect(boost::bind(&NRemoteFSBrowser::reply_list_favorites, this, _1));
  regist_signal("copy_request")
      .connect(boost::bind(&NRemoteFSBrowser::reply_copy_request, this, _1));
}

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::~NRemoteFSBrowser()
{
  delete m_completion_model;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::reply_read_dir ( common::SignalArgs & args )
{
  SignalOptions options( args );
  QStringList completion_list;
  int i;

  std::vector<std::string> dirs;
  std::vector<std::string> files;
  std::vector<std::string> dir_dates;
  std::vector<std::string> file_dates;
  std::vector<Uint> file_sizes;

  std::vector<std::string>::const_iterator it_dirs;
  std::vector<std::string>::const_iterator it_files;

  m_current_path = options.value<std::string>("dirPath").c_str();

  // add an ending '/' if the string does not have any
  if( !m_current_path.endsWith("/") )
    m_current_path += "/";

//  if(!m_updatingCompletion)
//    m_editPath->setText(m_current_path);
//  else
//    m_updatingCompletion = false;

  dirs = options.array<std::string>("dirs");
  files = options.array<std::string>("files");
  dir_dates = options.array<std::string>("dirDates");
  file_dates = options.array<std::string>("fileDates");
  file_sizes = options.array<Uint>("fileSizes");

  // notice the view(s) that the model is about to be completely changed
  emit layoutAboutToBeChanged();

  //clear();
  m_completion_model->setStringList( QStringList() );

  // delete old data
  while( !m_data.empty() )
    delete m_data.takeFirst();

  // add the directories
  for( i = 0, it_dirs = dirs.begin() ; it_dirs != dirs.end() ; ++it_dirs, ++i )
  {
    FileInfo * fileInfo = new FileInfo();
    QString name = it_dirs->c_str();

    fileInfo->name = name;
    fileInfo->type = DIRECTORY;

    fileInfo->date_modified = ""; //QString::fromStdString(dir_dates[i]);

    if(!m_current_path.isEmpty() && name != "..")
      name.prepend(m_current_path + (m_current_path.endsWith("/") ? "" : "/"));

    m_data.append(fileInfo);
    completion_list << name;
  }

  m_completion_model->setStringList( completion_list );

  // add the files
  for( i = 0, it_files = files.begin() ; it_files != files.end() ; ++it_files, ++i )
  {
    FileInfo * fileInfo = new FileInfo();

    fileInfo->name = it_files->c_str();
    fileInfo->type = FILE;
    fileInfo->date_modified = file_dates[i].c_str();
    fileInfo->file_size = file_sizes[i];

    m_data.append(fileInfo);
  }

  // notice the view(s) that the model has finished changing
  emit layoutChanged();
  emit current_path_changed(m_current_path);
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::reply_list_favorites ( SignalArgs &node )
{

  // temporarily not supported

//  std::vector<std::string> favs = node.get_array<std::string>("favorite_dirs");
//  std::vector<std::string>::iterator it = favs.begin();
  QStringList fav_dirs;

//  for(int i = 0 ; it != favs.end() ; ++it, ++i )
//    fav_dirs.append( QString::fromStdString(*it) );

  fav_dirs.removeDuplicates();

  emit favorites_changed( fav_dirs );
//  m_favorites_model->setStringList(fav_dirs);
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::copy_request ( std::vector<std::string> & parameters ){

  SignalFrame frame("copy_request", uri(), SERVER_CORE_PATH);
  frame.set_array("parameters", common::class_name<std::string>(), common::option_vector_to_str(parameters, ";"), ";");
  frame.options().flush();
  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::reply_copy_request ( common::SignalArgs & node ){

  emit copy_finished();
}

/////////////////////////////////////////////////////////////////////////////

const QStringListModel * NRemoteFSBrowser::completion_model() const
{
  return m_completion_model;
}

/////////////////////////////////////////////////////////////////////////////

QStringListModel * NRemoteFSBrowser::completion_model()
{
  return m_completion_model;
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::current_path() const
{
  return m_current_path;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::open_dir ( const QString & path )
{
  SignalFrame frame("read_dir", uri(), SERVER_CORE_PATH);
  SignalOptions options( frame );

  std::vector<std::string> vect;
  QStringList::iterator it = m_extensions.begin();

  while(it != m_extensions.end())
  {
    vect.push_back(it->toStdString());
    it++;
  }

  options.add("dirPath", path.toStdString());
  options.add("includeFiles", m_include_files);
  options.add("includeNoExtensions", m_include_no_extensions);
  options.add("extensions", vect);

  options.flush();

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::open_special_dir ( const QString & path )
{
  SignalFrame frame("read_special_dir", uri(), SERVER_CORE_PATH);
  SignalOptions options( frame );

  std::vector<std::string> vect;
  QStringList::iterator it = m_extensions.begin();

  while(it != m_extensions.end())
  {
    vect.push_back(it->toStdString());
    it++;
  }

  options.add("dirPath", path.toStdString());
  options.add("includeFiles", m_include_files);
  options.add("includeNoExtensions", m_include_no_extensions);
  options.add("extensions", vect);

  options.flush();

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::set_extensions( const QStringList & list )
{
  m_extensions = list;

  m_extensions.removeDuplicates();
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::add_extension( const QString & extension )
{
  if( !m_extensions.contains( extension ) )
    m_extensions.append( extension );
}

/////////////////////////////////////////////////////////////////////////////

QStringList NRemoteFSBrowser::extensions() const
{
  return m_extensions;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::set_include_no_extensions ( bool include )
{
  m_include_no_extensions = include;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::include_no_extensions () const
{
  return m_include_no_extensions;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::set_include_files ( bool include_files )
{
  m_include_files = include_files;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::include_files () const
{
  return m_include_files;
}

/////////////////////////////////////////////////////////////////////////////

QVariant NRemoteFSBrowser::data ( const QModelIndex & index, int role ) const
{
  QVariant data;

  if( index.isValid() )
  {
    FileInfo * item = m_data.at(index.row());

    if( role == Qt::DisplayRole )
    {
      switch( index.column() )
      {
      case 0:                     // item name
        data = item->name;
        break;

      case 1:                      // size (in case it's a file)
        data = is_file(index) ? size_to_string(item->file_size) : QString();
        break;

      case 2:                      // date modified
        data = item->date_modified;
        break;
      }
    }
    else if( role == Qt::TextAlignmentRole && index.column() == 1)
      return Qt::AlignRight;
    else if( role == Qt::ToolTipRole )
    {
      if(item->type == FILE)
        return QString("Size: %1\nDate modified: %2")
            .arg(size_to_string(item->file_size)).arg(item->date_modified);
      else
        return QString("Date modified: %1").arg(item->date_modified);
    }
  }

  return data;
}

/////////////////////////////////////////////////////////////////////////////

QModelIndex NRemoteFSBrowser::parent ( const QModelIndex & child ) const
{
  return QModelIndex();
}

/////////////////////////////////////////////////////////////////////////////

QModelIndex NRemoteFSBrowser::index ( int row, int column, const QModelIndex & parent ) const
{
  QModelIndex index;

  if( !parent.isValid() && this->hasIndex(row, column, parent) )
  {
    index = createIndex(row, column, m_data.at(row) );
  }

  return index;
}

/////////////////////////////////////////////////////////////////////////////

int NRemoteFSBrowser::rowCount(const QModelIndex &parent) const
{
  return m_data.count();
}

/////////////////////////////////////////////////////////////////////////////

int NRemoteFSBrowser::columnCount(const QModelIndex &parent) const
{
  return m_headers.count();
}

/////////////////////////////////////////////////////////////////////////////

QVariant NRemoteFSBrowser::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant header;

  if( role == Qt::DisplayRole && orientation == Qt::Horizontal &&
      section >= 0 && section < m_headers.count() )
      header = m_headers.at( section );

  return header;
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::retrieve_full_path( const QModelIndex & index ) const
{
  QString path;

  if( index.isValid() )
    path = m_current_path + data(index, Qt::DisplayRole).toString();

  return path;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::is_directory(const QModelIndex &index) const
{
  FileInfo * info = static_cast<FileInfo*>( index.internalPointer() );

  return is_not_null(info) && info->type == DIRECTORY;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::is_file(const QModelIndex &index) const
{
  FileInfo * info = static_cast<FileInfo*>( index.internalPointer() );

  return is_not_null(info) && info->type == FILE;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::update_favorite_list() const
{
  SignalFrame frame("list_favorites", uri(), SERVER_CORE_PATH);

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::send_favorites(const QStringList &favs)
{
  SignalFrame frame("set_favorites", uri(), SERVER_CORE_PATH);
  QStringList::const_iterator it = favs.begin();
  std::vector<std::string> vect( favs.count() );

  for( int i = 0 ; it != favs.end() ; ++it, ++i )
    vect[i] = it->toStdString();

  frame.set_array("favorite_dirs", common::class_name<std::string>(), common::option_vector_to_str(vect, ";"), ";");

  frame.options().flush();

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

Qt::ItemFlags NRemoteFSBrowser::flags(const QModelIndex &index) const
{
  return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled
                       | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::size_to_string( Uint size ) const
{
  Uint division_step = 1024;

  int iter = 0;
  QStringList units;
  Real divided_size = size;

  units << "bytes" << "KB" << "MB" << "GB" << "TB";

  while( divided_size > division_step )
  {
    divided_size /= division_step;
    iter++;
  }

  return QString::number( divided_size, 'f', 2) + ' ' + units[iter];
}

/////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
