// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NetworkQueue.hpp"

#include "UI/Core/NRemoteFSBrowser.hpp"

#include "Common/XML/FileOperations.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::NRemoteFSBrowser( const std::string & name ) :
  CNode(name, "NRemoteFSBrowser", CNode::DEBUG_NODE),
  m_includeNoExtensions(false),
  m_includeFiles(true)
{
  m_completionModel = new QStringListModel(this);

  m_headers << "Name" << "Size" << "Date Modified";

  regist_signal("read_dir")
      ->connect(boost::bind(&NRemoteFSBrowser::signal_read_dir, this, _1));

}

/////////////////////////////////////////////////////////////////////////////

NRemoteFSBrowser::~NRemoteFSBrowser()
{
  delete m_completionModel;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::signal_read_dir ( Common::SignalArgs & args )
{
  SignalOptions options( args );
  QStringList completionList;
  int i;

  std::vector<std::string> dirs;
  std::vector<std::string> files;
  std::vector<std::string> dirDates;
  std::vector<std::string> fileDates;
  std::vector<Uint> fileSizes;

  std::vector<std::string>::const_iterator itDirs;
  std::vector<std::string>::const_iterator itFiles;

  m_currentPath = options.value<std::string>("dirPath").c_str();

  // add an ending '/' if the string does not have any
  if( !m_currentPath.endsWith("/") )
    m_currentPath += "/";

//  if(!m_updatingCompletion)
//    m_editPath->setText(m_currentPath);
//  else
//    m_updatingCompletion = false;

  dirs = options.array<std::string>("dirs");
  files = options.array<std::string>("files");
  dirDates = options.array<std::string>("dirDates");
  fileDates = options.array<std::string>("fileDates");
  fileSizes = options.array<Uint>("fileSizes");

  // notice the view(s) that the model is about to be completely changed
  emit layoutAboutToBeChanged();

  //clear();
  m_completionModel->setStringList( QStringList() );

  // delete old data
  while( !m_data.empty() )
    delete m_data.takeFirst();

  // add the directories
  for( i = 0, itDirs = dirs.begin() ; itDirs != dirs.end() ; ++itDirs, ++i )
  {
    FileInfo * fileInfo = new FileInfo();
    QString name = itDirs->c_str();

    fileInfo->name = name;
    fileInfo->type = DIRECTORY;

    fileInfo->dateModified = dirDates[i].c_str();

    if(!m_currentPath.isEmpty() && name != "..")
      name.prepend(m_currentPath + (m_currentPath.endsWith("/") ? "" : "/"));

    m_data.append(fileInfo);
    completionList << name;
  }

  m_completionModel->setStringList( completionList );

  // add the files
  for( i = 0, itFiles = files.begin() ; itFiles != files.end() ; ++itFiles, ++i )
  {
    FileInfo * fileInfo = new FileInfo();

    fileInfo->name = itFiles->c_str();
    fileInfo->type = FILE;
    fileInfo->dateModified = fileDates[i].c_str();
    fileInfo->fileSize = fileSizes[i];

    m_data.append(fileInfo);
  }

  // notice the view(s) that the model has finished changing
  emit layoutChanged();
  emit currentPathChanged(m_currentPath);
}

/////////////////////////////////////////////////////////////////////////////

QStringListModel * NRemoteFSBrowser::completionModel() const
{
  return m_completionModel;
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::currentPath() const
{
  return m_currentPath;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::openDir ( const QString & path )
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

  options.add_option< OptionT<std::string> >("dirPath", path.toStdString());
  options.add_option< OptionT<bool> >("includeFiles", m_includeFiles);
  options.add_option< OptionT<bool> >("includeNoExtensions", m_includeNoExtensions);
  options.add_option< OptionArrayT<std::string> >("extensions", vect);

  options.flush();

  NetworkQueue::global_queue()->send( frame, NetworkQueue::IMMEDIATE );
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::setExtensions( const QStringList & list )
{
  m_extensions = list;

  m_extensions.removeDuplicates();
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::addExtension( const QString & extension )
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

void NRemoteFSBrowser::setIncludeNoExtensions ( bool include )
{
  m_includeNoExtensions = include;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::includeNoExtensions () const
{
  return m_includeNoExtensions;
}

/////////////////////////////////////////////////////////////////////////////

void NRemoteFSBrowser::setIncludeFiles ( bool includeFiles )
{
  m_includeFiles = includeFiles;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::includeFiles () const
{
  return m_includeFiles;
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
      case 0 :                     // item name
        data = item->name;
        break;

      case 1:                      // size (in case it's a file)
        data = isFile(index) ? sizeToString(item->fileSize) : QString();
        break;

      case 2:                      // date modified
        data = item->dateModified;
        break;
      }
    }
    else if( role == Qt::TextAlignmentRole && index.column() == 1)
      return Qt::AlignRight;
    else if( role == Qt::ToolTipRole )
    {
      if(item->type == FILE)
        return QString("Size: %1\nDate modified: %2")
            .arg(sizeToString(item->fileSize)).arg(item->dateModified);
      else
        return QString("Date modified: %1").arg(item->dateModified);
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

QString NRemoteFSBrowser::retrieveFullPath( const QModelIndex & index ) const
{
  QString path;

  if( index.isValid() )
    path = m_currentPath + data(index, Qt::DisplayRole).toString();

  return path;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::isDirectory(const QModelIndex &index) const
{
  FileInfo * info = static_cast<FileInfo*>( index.internalPointer() );

  return is_not_null(info) && info->type == DIRECTORY;
}

/////////////////////////////////////////////////////////////////////////////

bool NRemoteFSBrowser::isFile(const QModelIndex &index) const
{
  FileInfo * info = static_cast<FileInfo*>( index.internalPointer() );

  return is_not_null(info) && info->type == FILE;
}

/////////////////////////////////////////////////////////////////////////////

QString NRemoteFSBrowser::sizeToString( Uint size ) const
{
  Uint divisionStep = 1024;

  int iter = 0;
  QStringList units;
  Real dividedSize = size;

  units << "bytes" << "KB" << "MB" << "GB" << "TB";

  while( dividedSize > divisionStep )
  {
    dividedSize /= divisionStep;
    iter++;
  }

  return QString::number( dividedSize, 'f', 2) + ' ' + units[iter];
}

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
