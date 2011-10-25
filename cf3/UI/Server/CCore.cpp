// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtXml>
#include <QHostInfo>

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


#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Server/RemoteClientAppender.hpp"
#include "UI/Server/ServerNetworkComm.hpp"
#include "UI/Server/ServerRoot.hpp"

#include "UI/Server/CCore.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::UICommon;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Server {

/////////////////////////////////////////////////////////////////////////////

CCore::CCore()
  : Component(SERVER_CORE),
    DEFAULT_PATH("."),
    m_fileOpen(false),
    m_simRunning(false),
    m_active(false)
{


  TypeInfo::instance().regist<CCore>( type_name() );

  m_commServer = new ServerNetworkComm();

  connect(m_commServer, SIGNAL(newClientConnected(std::string)),
          this,  SLOT(newClient(std::string)));

  RemoteClientAppender * rca = new RemoteClientAppender();

  Logger::instance().getStream(WARNING).addStringForwarder(rca);
  Logger::instance().getStream(ERROR).addStringForwarder(rca);
  Logger::instance().getStream(INFO).addStringForwarder(rca);

  Logger::instance().getStream(INFO).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(ERROR).setStamp(LogStream::STRING, "%type% ");
  Logger::instance().getStream(WARNING).setStamp(LogStream::STRING, "%type% ");

  connect(rca, SIGNAL(newData(QString)), this, SLOT(message(QString)));

  regist_signal( "read_dir" )
    ->description("Read directory content")
    ->read_only(true)
    ->pretty_name("")->connect(boost::bind(&CCore::read_dir, this, _1));
  regist_signal( "shutdown" )
    ->description("Shutdown the server")
    ->pretty_name("")->connect(boost::bind(&CCore::shutdown, this, _1));
}

/////////////////////////////////////////////////////////////////////////////

CCore::~CCore()
{
  delete m_commServer;
}

/////////////////////////////////////////////////////////////////////////////

bool CCore::listenToPort(quint16 portNumber)
{
  return m_commServer->openPort(portNumber);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::sendSignal( const XmlDoc & signal )
{
  m_commServer->sendSignalToClient(signal);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::sendFrameRejected(const std::string & clientid,
                              const std::string & frameid,
                              const cf3::common::URI & sender,
                              const QString & reason)
{
  m_commServer->sendFrameRejectedToClient(clientid, frameid, sender, reason);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::sendException(const char * what,
                          const std::string & clientid)
{
  m_commServer->sendMessageToClient(what, LogMessage::EXCEPTION, clientid);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::forward_signal( SignalArgs & args )
{
  sendSignal( *args.xml_doc );
}

/***************************************************************************

PRIVATE METHODS

***************************************************************************/

bool CCore::getDirContent(const QString & directory,
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
          content.dirDates.push_back(modified.toStdString());
        }
        else if(includeFiles)
        {
          if(regex.exactMatch(filename) ||
             (includeNoExtension && !filename.contains('.')) )
          {
            content.files.push_back(filename.toStdString());
            content.fileDates.push_back(modified.toStdString());
            content.fileSizes.push_back(fileInfo.size());
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
  std::string clientId( attr != nullptr ? attr->value() : "" );


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

  content.dirDates.push_back( QFileInfo("..").lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString() );

  if(!this->getDirContent(directory, exts, includeFiles, includeNoExt, content))
  {
    throw FileSystemError(FromHere(), dirPath.toStdString() + ": no such direcrory");
  }
  else
  {
    // Build the reply
    SignalFrame reply = args.create_reply( uri() );
    SignalOptions roptions = reply.options();

    roptions.add_option<OptionT<std::string> >("dirPath", directory.toStdString());
    roptions.add_option<OptionArrayT<std::string> >("dirs", content.dirs);
    roptions.add_option<OptionArrayT<std::string> >("files", content.files);
    roptions.add_option<OptionArrayT<std::string> >("dirDates", content.dirDates);
    roptions.add_option<OptionArrayT<std::string> >("fileDates", content.fileDates);
    roptions.add_option<OptionArrayT<Uint> >("fileSizes", content.fileSizes);

    roptions.flush();

  }
}

/////////////////////////////////////////////////////////////////////////////

void CCore::newEvent(const std::string & name, const URI & path)
{
  SignalFrame frame(name, path, path);

  m_commServer->sendSignalToClient(*frame.xml_doc.get());
}

/////////////////////////////////////////////////////////////////////////////

void CCore::createDir(SignalArgs & node)
{
  m_commServer->sendMessageToClient("Cannot create a directory yet", LogMessage::ERROR);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::shutdown(SignalArgs & node)
{
  ServerRoot::instance().manager()->kill_group("Workers");
  qApp->exit(0); // exit the Qt event loop
}

/////////////////////////////////////////////////////////////////////////////

void CCore::saveConfig(SignalArgs & node)
{
  m_commServer->sendMessageToClient("Cannot save the configuration yet", LogMessage::ERROR);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::newClient(const std::string & clientId)
{
  // send a welcome message to the new client
  m_commServer->sendMessageToClient("Welcome to the Client-Server project!", LogMessage::INFO, clientId);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::message(const QString & message)
{
  QString typeStr = message.split(" ").first();
  QString copy(message);
  LogMessage::Type type = LogMessage::Convert::instance().to_enum(typeStr.toStdString());

  m_commServer->sendMessageToClient(copy.remove(0, typeStr.length() + 1), type);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::error(const QString & message)
{
  m_commServer->sendMessageToClient(message, LogMessage::ERROR);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::sendACK( const std::string & clientid,
                     const std::string & frameid,
                     bool success,
                     const std::string & message)
{
  SignalFrame frame("ack", uri(), CLIENT_NETWORK_QUEUE_PATH);
  SignalOptions & options = frame.options();


  options.add_option< OptionT<std::string> >("frameid", frameid );
  options.add_option< OptionT<bool> >("success", success );
  options.add_option< OptionT<std::string> >("message", message );

  options.flush();

  m_commServer->sendSignalToClient( *frame.xml_doc.get(), clientid);
}

/////////////////////////////////////////////////////////////////////////////

} // Server
} // UI
} // cf3
