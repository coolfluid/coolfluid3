// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <QApplication>
//#include <QDateTime>
//#include <QHostInfo>
//#include <QSettings>
//#include <QDir>

#include <ctime>

#include "coolfluid-config.hpp"
#if defined CF3_OS_LINUX || CF3_OS_MACOSX // if we are on a POSIX system...
  #include <pwd.h> // for getpwuid()
#endif

#include <boost/algorithm/string/join.hpp>
#include <boost/regex.hpp>

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

/// Finds the home directory path of the current user.
/// @return Return the home directory path.
/// @throw ValueNotFound If the home directory path could not be retrieved.
/// @note This function was written on Linux and was tested on Linux and OSX.
/// There are @b NO garantee on its behavior on Windows as it was never
/// officially tested on that operating system.
std::string find_home_path()
{
  std::string home_path;

#if defined CF3_OS_LINUX || CF3_OS_MACOSX // if we are on a POSIX system...
  char * home = getenv("HOME");

  if( is_not_null(home) )
    home_path = home;
  else
  {
    // the $HOME environment variable may not exist (i.e. the application
    // was not lauched from a terminal on OSX). We need to use getpwuid()
    // function then.
    passwd * pwd = getpwuid( getuid() );

    if( is_not_null(pwd) )
      home_path = pwd->pw_dir; // get home directory
  }
#else
#ifdef CF3_OS_WINDOWS
 char * user_profile = getenv("USERPROFILE");

 if( is_not_null(user_profile) )
   home_path = user_profile;

 // note: some WebSites recommend to use %HOMEDRIVE% + %HOMEPATH% if
 // %USERPROFILE% does not exist. This does not seem a good solution to me
 // because - according to Microsoft documentation - %HOMEDRIVE% may not
 // contain a useful value in the case of a network home directory (in which
 // case %HOMESHARE% should also have to be used somewhere).
#endif
#endif

  if( home_path.empty() )
    throw ValueNotFound( FromHere(), "Could not retrieve home directory." );

  return home_path;
}

/////////////////////////////////////////////////////////////////////////////

CCore::CCore()
  : Component(SERVER_CORE),
    default_path(".")
{
  TypeInfo::instance().regist<CCore>( type_name() );

  m_comm_server = new ServerNetworkComm();
//  m_settings = new QSettings( "vki.ac.be", "coolfluid-server" );

//  m_favorite_directories = m_settings->value( "favorite_directories", QVariant(QStringList()) ).toStringList();

  m_comm_server->signal( "new_client_connected" )
      ->connect(boost::bind(&CCore::new_client, this, _1));

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
  return m_comm_server->open_port(portNumber);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::send_signal( SignalFrame & signal )
{
  m_comm_server->send_frame_to_client(signal);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::forward_signal( SignalArgs & args )
{
  send_signal( args );
}

/***************************************************************************

                           PRIVATE METHODS

***************************************************************************/

bool CCore::get_dir_content( const std::string &directory,
                             const std::vector<std::string> & extensions,
                             bool include_files,
                             bool include_no_ext,
                             DirContent & content) const
{
  using namespace boost::filesystem3;

  path p(directory);
  bool dir_exists = exists(p) && is_directory(p);

  if( dir_exists )
  {
    boost::regex regexp(".*\\..*"); // matches file that have an extension

    if( include_files && !extensions.empty() )
    {
      /* build the regex pattern string.
         For example, if the vector contains "xml" and "CFcase" extensions,
         the resulting string will be : "^.+\\.((xml)|(CFcase))$"
      */

      std::string pattern = boost::algorithm::join( extensions, ")|(" );

      pattern = "^.+\\.((" + pattern + "))$";
      regexp = boost::regex( pattern );
    }

    // iterate through the directory contents
    for( directory_iterator it_dir(p) ; it_dir != directory_iterator() ; ++it_dir )
    {
      directory_entry entry = *it_dir;
      path entry_path = entry.path();
      std::string filename = entry_path.filename().string();
      std::time_t modif_time = last_write_time(entry_path);
      std::string time_str( std::ctime(&modif_time) );

      time_str.resize( time_str.length()-1 ); // remove the ending '\n'

      if( is_directory( entry_path ) )
      {
        content.dirs.push_back( filename );
        content.dir_dates.push_back( time_str );
      }
      else if ( is_regular_file( entry_path ) && include_files )
      {
        // (1) filename matches the regular expression
        // (2) files w/out extension are allowed and filename doesn't have one
        if( boost::regex_match( filename, regexp ) || // (1)
            ( include_no_ext && !entry_path.has_extension() ) ) // (2)
        {
          content.files.push_back( filename );
          content.file_sizes.push_back( file_size(entry_path) );
          content.file_dates.push_back( time_str );
        }
      }
    }

  }

  return dir_exists;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                               BOOST SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::read_dir(SignalArgs & args)
{
  SignalOptions options( args );

  DirContent content;
  std::string directory;

  std::string dirPath = options.value<std::string>("dirPath");
  bool includeFiles = options.value<bool>("includeFiles");
  bool includeNoExt = options.value<bool>("includeNoExtensions");
  std::vector<std::string> exts = options.array<std::string>("extensions");

  if(dirPath.empty())
    directory = this->default_path;
  else
    directory = dirPath;

  // get the absolute path
  directory = boost::filesystem3::complete(directory).string();
//  directory = QDir::cleanPath(directory.c_str()).toStdString();

  // if the directory is not the root
  /// @todo test this on Windows!!!!
  if(directory != "/")
    content.dirs.push_back("..");

  content.dir_dates.push_back("");//QFileInfo("..").lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString() );

  if(! get_dir_content( directory, exts, includeFiles, includeNoExt, content ) )
  {
    throw FileSystemError(FromHere(), dirPath + ": no such directory");
  }
  else
  {
    // Build the reply
    SignalFrame reply = args.create_reply( uri() );
    SignalOptions roptions = reply.options();

    roptions.add_option("dirPath", directory);
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

  DirContent content;
  std::string directory;

  std::string dir_path = options.value<std::string>("dirPath");
  bool includeFiles = options.value<bool>("includeFiles");
  bool includeNoExt = options.value<bool>("includeNoExtensions");
  std::vector<std::string> exts = options.array<std::string>("extensions");

//  if(dirPath.isEmpty())
//    directory = this->DEFAULT_PATH;
//  else
    directory = dir_path;

  if( directory == "Home" )
    directory = find_home_path();//QDir::homePath().toStdString();
  else
    throw ValueNotFound( FromHere(),
                         "Unknown special directory [" + directory + "]." );

  // get the absolute path
  directory = boost::filesystem3::complete(directory).string();

  // if the directory is not the root
  /// @todo test this on Windows!!!!
  if(directory != "/")
    content.dirs.push_back("..");

  content.dir_dates.push_back("");// QFileInfo("..").lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString() );

  if(! get_dir_content( directory, exts, includeFiles, includeNoExt, content ) )
  {
    throw FileSystemError(FromHere(), dir_path + ": no such direcrory");
  }
  else
  {
    // Build the reply
    SignalFrame reply = args.create_reply( uri() );
    SignalOptions roptions = reply.options();

    reply.node.set_attribute( "target", "read_dir" );

    roptions.add_option("dirPath", directory);
    roptions.add_option("dirs", content.dirs);
    roptions.add_option("files", content.files);
    roptions.add_option("dirDates", content.dir_dates);
    roptions.add_option("fileDates", content.file_dates);
    roptions.add_option("fileSizes", content.file_sizes);

    roptions.flush();

  }
}

/////////////////////////////////////////////////////////////////////////////

void CCore::shutdown( SignalArgs & )
{
  ServerRoot::instance().manager()->kill_group("Workers");
  m_comm_server->close(); // stop the network thread
}

/////////////////////////////////////////////////////////////////////////////

void CCore::signal_set_favorites(SignalArgs &node)
{
  std::vector<std::string> favs = node.get_array<std::string>("favorite_dirs");
  std::vector<std::string>::iterator it = favs.begin();

//  m_favorite_directories.clear();

//  for(int i = 0 ; it != favs.end() ; ++it, ++i )
//    m_favorite_directories.append( QString::fromStdString(*it) );

//  m_favorite_directories.removeDuplicates();

//  m_settings->setValue( "favorite_directories", m_favorite_directories );
//  m_settings->sync();
}

/////////////////////////////////////////////////////////////////////////////

void CCore::signal_list_favorites( SignalArgs &node )
{
  SignalArgs reply = node.create_reply();
//  QStringList::iterator it = m_favorite_directories.begin();
//  std::vector<std::string> vect( m_favorite_directories.size() );

//  for( int i = 0 ; it != m_favorite_directories.end() ; ++it, ++i )
//    vect[i] = it->toStdString();

//  reply.set_array<std::string>("favorite_dirs", vect, ";");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::new_client( SignalArgs & args)
{
  // send a welcome message to the new client
  std::string clientid = args.options().value<std::string>("clientid");
  m_comm_server->send_message_to_client("Welcome to the Client-Server project!", LogMessage::INFO, clientid);
}

/////////////////////////////////////////////////////////////////////////////

void CCore::send_ack( const std::string & clientid,
                      const std::string & frameid,
                      bool success,
                      const std::string & message )
{
  SignalFrame frame("ack", uri(), CLIENT_NETWORK_QUEUE_PATH);
  SignalOptions & options = frame.options();


  options.add_option("frameid", frameid );
  options.add_option("success", success );
  options.add_option("message", message );

  options.flush();

  m_comm_server->send_frame_to_client( frame, clientid);
}

/////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3
