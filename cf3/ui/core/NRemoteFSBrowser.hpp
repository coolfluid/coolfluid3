// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NRemoteFSBrowser_hpp
#define cf3_ui_core_NRemoteFSBrowser_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QList>
#include <QSortFilterProxyModel>
#include <QStringListModel>

#include "ui/core/CNode.hpp"

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

class NRemoteFSBrowser :
    public QAbstractItemModel,
    public CNode
{
  Q_OBJECT

public: // typedefs

  
  

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

    QString date_modified;

    Uint file_size;

  }; // FileInfo

  NRemoteFSBrowser( const std::string & name );

  virtual ~NRemoteFSBrowser();

  static std::string type_name() { return "NRemoteFSBrowser"; }

  QStringListModel * completion_model();

  const QStringListModel * completion_model() const;

  QString current_path() const;

  void open_dir ( const QString & path );

  void open_special_dir ( const QString & path );

  void set_extensions( const QStringList & list );

  void add_extension( const QString & extension );

  QStringList extensions() const;

  void set_include_no_extensions ( bool include );

  bool include_no_extensions () const;

  void set_include_files ( bool include_files );

  bool include_files () const;

  virtual QVariant data ( const QModelIndex & index, int role ) const;

  virtual QModelIndex parent ( const QModelIndex & child ) const;

  virtual QModelIndex index ( int row, int column, const QModelIndex & parent ) const;

  virtual int rowCount ( const QModelIndex &parent ) const;

  virtual int columnCount ( const QModelIndex &parent ) const;

  virtual QString tool_tip() const {return component_type(); }

  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role  = Qt::DisplayRole ) const;


  virtual void disable_local_signals(QMap<QString, bool>&) const {}

  QString retrieve_full_path( const QModelIndex & index ) const;

  bool is_file( const QModelIndex & index ) const;

  bool is_directory( const QModelIndex & index ) const;

  void update_favorite_list() const;

  void send_favorites( const QStringList & favs );

  /// @name Signals
  //@{

  void reply_read_dir ( common::SignalArgs & node );

  void reply_list_favorites ( common::SignalArgs & node );

  // @} END Signals

  void copy_request (std::vector<std::string> & parameters);

  void reply_copy_request ( common::SignalArgs & node );

protected:

  Qt::ItemFlags flags(const QModelIndex &index) const;

signals:

  void current_path_changed( const QString & newPath );

  void favorites_changed( const QStringList & list );

  void copy_finished ();

private: // functions

  QString size_to_string( Uint size ) const;

private: // data

  QList<FileInfo*> m_data;

  QString m_current_path;

  QStringListModel * m_completion_model;

  QStringList m_extensions;

  QStringList m_headers;

  bool m_include_no_extensions;

  bool m_include_files;

  bool m_updating_completion;

}; // NRemoteFSBrowser

/////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_NRemoteFSBrowser_hpp
