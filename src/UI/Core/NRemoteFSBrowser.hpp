// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UI_Core_NRemoteFSBrowser_hpp
#define cf3_UI_Core_NRemoteFSBrowser_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "UI/Core/CNode.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

/////////////////////////////////////////////////////////////////////////////

class NRemoteFSBrowser :
    public QAbstractItemModel,
    public CNode
{
  Q_OBJECT

public: // typedefs

  typedef boost::shared_ptr<NRemoteFSBrowser> Ptr;
  typedef boost::shared_ptr<const NRemoteFSBrowser> ConstPtr;

public:

  enum FileType
  {
    FILE = 0,

    DIRECTORY = 1

  }; // FileType

  struct FileInfo
  {
    QString name;

    FileType type;

    QString dateModified;

    Uint fileSize;

  }; // FileInfo

  NRemoteFSBrowser( const std::string & name );

  virtual ~NRemoteFSBrowser();

  static std::string type_name() { return "NRemoteFSBrowser"; }

  QStringListModel * completionModel() const;

  QString currentPath() const;

  void openDir ( const QString & path );

  void setExtensions( const QStringList & list );

  void addExtension( const QString & extension );

  QStringList extensions() const;

  void setIncludeNoExtensions ( bool include );

  bool includeNoExtensions () const;

  void setIncludeFiles ( bool includeFiles );

  bool includeFiles () const;

  virtual QVariant data ( const QModelIndex & index, int role ) const;

  virtual QModelIndex parent ( const QModelIndex & child ) const;

  virtual QModelIndex index ( int row, int column, const QModelIndex & parent ) const;

  virtual int rowCount ( const QModelIndex &parent ) const;

  virtual int columnCount ( const QModelIndex &parent ) const;

  virtual QString toolTip() const {return componentType(); }

  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role  = Qt::DisplayRole ) const;


  virtual void disableLocalSignals(QMap<QString, bool>&) const {}

  QString retrieveFullPath( const QModelIndex & index ) const;

  bool isFile( const QModelIndex & index ) const;

  bool isDirectory( const QModelIndex & index ) const;

  /// @name Signals
  //@{

  void signal_read_dir ( common::SignalArgs & node );

  // @} END Signals

signals:

  void currentPathChanged( const QString & newPath );

private: // functions

  QString sizeToString( Uint size ) const;

private: // data

  QList<FileInfo*> m_data;

  QString m_currentPath;

  QStringListModel * m_completionModel;

  QStringList m_extensions;

  QStringList m_headers;

  bool m_includeNoExtensions;

  bool m_includeFiles;

  bool m_updatingCompletion;

}; // NRemoteFSBrowser

/////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // CF3_UI_Core_NRemoteFSBrowser_hpp
