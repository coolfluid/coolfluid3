// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QApplication>
#include <QDateTime>
#include <QHostInfo>
#include <QSettings>

#include "rapidxml/rapidxml.hpp"

#include "common/BoostFilesystem.hpp"

#include "common/BasicExceptions.hpp"
#include "common/PE/Comm.hpp"
#include "common/Log.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/PE/Manager.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"


#include "ui/uicommon/ComponentNames.hpp"

#include "ui/server/ServerNetworkComm.hpp"
#include "ui/server/ServerRoot.hpp"

#include "ui/server/CCore.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::uiCommon;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {

/////////////////////////////////////////////////////////////////////////////

CCore::CCore()
  : Component(SERVER_CORE),
    DEFAULT_PATH(".")
{
  TypeInfo::instance().regist<CCore>( type_name() );

  m_comm_server = new ServerNetworkComm();
  m_settings = new QSettings( "vki.ac.be", "coolfluid-server", this);

  m_favorite_directories = m_settings->value( "favorite_directories", QVariant(QStringList()) ).toStringList();

  connect( m_comm_server, SIGNAL(newClientConnected(std::string)),
           this,  SLOT(new_client(std::string)) );

  Logger::instance().getStream(INFO).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(ERROR).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(WARNING).setStamp(LogStream::STRING, "%type% ");

  regist_signal( "read_dir" )
    .description("Read directory content")
    .read_only(true)
    .pretty_name("").connect(boost::bind(&CCore::read_dir, this, _1));

  regist_signal( "read_special_dir" )
    .description("Read special directory content")
    .read_only(true)
    .pretty_name("").connect(boost::bind(&CCore::read_special_dir, this, _1));

  regist_signal( "shutdown" )
    .description("Shutdown the server")
    .pretty_name("").connect(boost::bind(&CCore::shutdown, this, _1));

  regist_signal("list_favorites")
      .description( "Lists the favorite directories for the remote browsing feature." )
      .read_only(true)
      .connect(boost::bind(&CCore::signal_list_favorites, this, _1));

  regist_signal("set_favorites")
      .description( "Sets the favorite directories for the remote browsing feature." )
      .read_only(true)
      .connect(boost::bind(&CCore::signal_set_favorites, this, _1));
}

/////////////////////////////////////////////////////////////////////////////

CCore::~CCore()
{
  delete m_comm_server;
}

/////////////////////////////////////////////////////////////////////////////

bool CCore::listen_to_port(quint16 portNumber)
{
  return m_comm_server->openPort(portNumber);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::send_signal( const XmlDoc & signal )
{
  m_comm_server->sendSignalToClient(signal);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::forward_signal( SignalArgs & args )
{
  send_signal( *args.xml_doc );
}

/***************************************************************************

PRIVATE METHODS

***************************************************************************/

bool CCore::get_dir_content(const QString & directory,
                          const std::vector<std::string> & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          DirContent & content) const {
  QDir dir(directory);
  bool dirExists = dir.exists();

  dir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
  dir.setSorting(QDir::DirsFirst | QDir::Name);

  if(dirExists)
  {
    QFileInfoList files = dir.entryInfoList();
    QFileInfoList::iterator it = files.begin();

    QRegExp regex("", Qt::CaseSensitive, QRegExp::RegExp);

    if(!extensions.empty())
    {
      /* build the regex pattern string.
        For example, if the QStringList contains "xml" and "CFcase" extensions,
        the resulting string will be : "^.+\\.((xml)|(CFcase))$" */

      /// @todo try to use QString::resize() or QString::reserve()
      QString regexPattern;
      std::vector<std::string>::const_iterator it = extensions.begin();

      while(it != extensions.end())
      {
        if(!regexPattern.isEmpty())
          regexPattern.append(")|(");

        regexPattern.append(it->c_str());

        it++;
      }

//      QString regexPattern = extensions.join(")|(");
      regexPattern.prepend("^.+\\.((").append("))$");
      regex.setPattern(regexPattern);
    }
    else
      regex.setPattern("^.+\\..+$");

    while(it != files.end())
    {
      QFileInfo fileInfo = *it;
      QString filename = fileInfo.fileName();
      QString modified = fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");

      if (filename != "." && filename != "..")
      {
        if(fileInfo.isDir())
        {
          content.dirs.push_back(filename.toStdString());
          content.dir_dates.push_back(modified.toStdString());
        }
        else if(includeFiles)
        {
          if(regex.exactMatch(filename) ||
             (includeNoExtension && !filename.contains('.')) )
          {
            content.files.push_back(filename.toStdString());
            content.file_dates.push_back(modified.toStdString());
            content.file_sizes.push_back(fileInfo.size());
          }
        }
      }

      it++;
    }
  }

  return dirExists;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                               BOOST SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::read_dir(SignalArgs & args)
{
  SignalOptions options( args );

//  std::vector<std::string> dirList;
//  std::vector<std::string> fileList;
  DirContent content;
  QString directory;
  rapidxml::xml_attribute<>* attr = args.node.content->first_attribute( "clientid" );

  QString dirPath = options.value<std::string>("dirPath").c_str();
  bool includeFiles = options.value<bool>("includeFiles");
  bool includeNoExt = options.value<bool>("includeNoExtensions");
  std::vector<std::string> exts = options.array<std::string>("extensions");

  if(dirPath.isEmpty())
    directory = this->DEFAULT_PATH;
  else
    directory = dirPath;

  directory = QDir(directory).absolutePath();
  directory = QDir::cleanPath(directory);

  // if the directory is not the root
  /// @todo test this on Windows!!!!
  if(directory != "/")
    content.dirs.push_back("..");

  content.dir_dates.push_back( QFileInfo("..").lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString() );

  if(!this->get_dir_content(directory, exts, includeFiles, includeNoExt, content))
  {
    throw FileSystemError(FromHere(), dirPath.toStdString() + ": no such direcrory");
  }
  else
  {
    // Build the reply
    SignalFrame reply = args.create_reply( uri() );
    SignalOptions roptions = reply.options();

    roptions.add_option("dirPath", directory.toStdString());
    roptions.add_option("dirs", content.dirs);
    roptions.add_option("files", content.files);
    roptions.add_option("dirDates", content.dir_dates);
    roptions.add_option("fileDates", content.file_dates);
    roptions.add_option("fileSizes", content.file_sizes);

    roptions.flush();

  }
}

/////////////////////////////////////////////////////////////////////////////

void CCore::read_special_dir(SignalArgs & args)
{
  SignalOptions options( args );

//  std::vector<std::string> dirList;
//  std::vector<std::string> fileList;
  DirContent content;
  QString directory;
  rapidxml::xml_attribute<>* attr = args.node.content->first_attribute( "clientid" );

  QString dirPath = options.value<std::string>("dirPath").c_str();
  bool includeFiles = options.value<bool>("includeFiles");
  bool includeNoExt = options.value<bool>("includeNoExtensions");
  std::vector<std::string> exts = options.array<std::string>("extensions");

//  if(dirPath.isEmpty())
//    directory = this->DEFAULT_PATH;
//  else
    directory = dirPath;

  if( directory == "Home" )
    directory = QDir::homePath();
  else
    throw ValueNotFound( FromHere(),
                         "Unknown special directory [" + directory.toStdString() + "]." );

//  directory = QDir(directory).absolutePath();
//  directory = QDir::cleanPath(directory);

  // if the directory is not the root
  /// @todo test this on Windows!!!!
  if(directory != "/")
    content.dirs.push_back("..");

  content.dir_dates.push_back( QFileInfo("..").lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString() );

  if(!this->get_dir_content(directory, exts, includeFiles, includeNoExt, content))
  {
    throw FileSystemError(FromHere(), dirPath.toStdString() + ": no such direcrory");
  }
  else
  {
    // Build the reply
    SignalFrame reply = args.create_reply( uri() );
    SignalOptions roptions = reply.options();

    reply.node.set_attribute( "target", "read_dir" );

    roptions.add_option("dirPath", directory.toStdString());
    roptions.add_option("dirs", content.dirs);
    roptions.add_option("files", content.files);
    roptions.add_option("dirDates", content.dir_dates);
    roptions.add_option("fileDates", content.file_dates);
    roptions.add_option("fileSizes", content.file_sizes);

    roptions.flush();

  }
}

/////////////////////////////////////////////////////////////////////////////

void CCore::shutdown(SignalArgs & node)
{
  ServerRoot::instance().manager()->kill_group("Workers");
  qApp->exit(0); // exit the Qt event loop
}

/////////////////////////////////////////////////////////////////////////////

void CCore::signal_set_favorites(SignalArgs &node)
{
  std::vector<std::string> favs = node.get_array<std::string>("favorite_dirs");
  std::vector<std::string>::iterator it = favs.begin();

  m_favorite_directories.clear();

  for(int i = 0 ; it != favs.end() ; ++it, ++i )
    m_favorite_directories.append( QString::fromStdString(*it) );

  m_favorite_directories.removeDuplicates();

  m_settings->setValue( "favorite_directories", m_favorite_directories );
  m_settings->sync();
}

/////////////////////////////////////////////////////////////////////////////

void CCore::signal_list_favorites( SignalArgs &node )
{
  SignalArgs reply = node.create_reply();
  QStringList::iterator it = m_favorite_directories.begin();
  std::vector<std::string> vect( m_favorite_directories.size() );

  for( int i = 0 ; it != m_favorite_directories.end() ; ++it, ++i )
    vect[i] = it->toStdString();

  reply.set_array<std::string>("favorite_dirs", vect, ";");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::new_client(const std::string & clientId)
{
  // send a welcome message to the new client
  m_comm_server->sendMessageToClient("Welcome to the Client-Server project!", LogMessage::INFO, clientId);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::send_ack( const std::string & clientid,
                     const std::string & frameid,
                     bool success,
                     const std::string & message)
{
  SignalFrame frame("ack", uri(), CLIENT_NETWORK_QUEUE_PATH);
  SignalOptions & options = frame.options();


  options.add_option("frameid", frameid );
  options.add_option("success", success );
  options.add_option("message", message );

  options.flush();

  m_comm_server->sendSignalToClient( *frame.xml_doc.get(), clientid);
}

/////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3
