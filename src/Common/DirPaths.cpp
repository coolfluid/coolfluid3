#include "Common/Log.hpp"
#include "Common/LogLevel.hpp"

#include "Common/DirPaths.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace CF::Common;

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

DirPaths::DirPaths() :
  m_baseDir(),
  m_modulesDir(),
  m_workinDir(),
  m_resultsDir(),
  m_repURL()
{
}

////////////////////////////////////////////////////////////////////////////////

DirPaths::~DirPaths()
{
//   CFLog(NOTICE, " ++++++ Destroying DirPaths ++++++ \n");
}

////////////////////////////////////////////////////////////////////////////////

DirPaths& DirPaths::instance()
{
  static DirPaths dirPaths;
  return dirPaths;
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::checkThrowMultipleBadDir(const std::vector< boost::filesystem::path >& paths)
{
  if (!paths.empty()) {
    std::string totalPaths;
    vector< boost::filesystem::path >::const_iterator itr = paths.begin();
    for(; itr != paths.end(); ++itr) {
      totalPaths += "\'"+itr->string()+"\'";
      totalPaths += std::string(" ");
    }
    throw FileSystemError (FromHere(),"Following paths are not directories : " + totalPaths);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::setDir(boost::filesystem::path& dpath, const std::string& dstr, const bool createdir)
{
  boost::filesystem::path d(dstr);

  if (!dstr.empty()) {
    if (createdir) {

      // The logic for directory creation is:
      // - path exists, not a directory: throw exception
      // - path exists, is a directory:  do nothing
      // - path does not exist:          create directory
      if (boost::filesystem::exists(d)) {
        if (!boost::filesystem::is_directory(d))  {
          throw FileSystemError(FromHere(),"Path \'"+dstr+"\'" + " exists and is not a directory");
        }
      } else {
        boost::filesystem::create_directory(d);
      }

    } else {

      if (!boost::filesystem::exists(d))  {
        throw FileSystemError(FromHere(),"Path \'"+dstr+"\'" + " does not exist");
      }

    }
  }
  dpath = d;
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::setBaseDir(const std::string& baseDir)
{
  CFinfo << "Base Dir set to: \'" << baseDir << "\'\n";
  setDir(m_baseDir,baseDir);
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::addModuleDirs(const vector<std::string>& modulesDir)
{
  vector< boost::filesystem::path > badPaths;

  vector<std::string>::const_iterator itr = modulesDir.begin();
  for(; itr != modulesDir.end(); ++itr) {
    boost::filesystem::path p (*itr);
    if(boost::filesystem::exists(p))
    {
      if (boost::filesystem::is_directory(p)) {
        m_modulesDir.push_back(p);
        CFinfo << "Adding Module Dir: " << "\'" << p.string() << "\'\n";
      }
      else {
        badPaths.push_back(p);
      }
    }
  }

  checkThrowMultipleBadDir(badPaths);
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::setWorkingDir(const std::string& workingDir)
{
  // if workingDir starts with "." or ".." then do not append m_baseDir
  boost::filesystem::path wDir;
  boost::filesystem::path workPath(workingDir);
  if (workPath.root_path().empty()) {
	CFinfo << "Found a relative working dir path\n";
    wDir = m_baseDir / workPath;
  }
  else {
    wDir = workPath;
    CFinfo << "Found an absolute working dir path\n";
  }

  CFinfo << "Working Dir set to: \'" << wDir.string() << "\'\n";
  setDir(m_workinDir,wDir.string());
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::setRepositoryURL(const std::string& url)
{
  m_repURL = url;
}

////////////////////////////////////////////////////////////////////////////////

void DirPaths::setResultsDir(const std::string& resultsDir)
{
  // if resultsDir starts with "." or ".." then do not append m_baseDir
  boost::filesystem::path rDir;
  boost::filesystem::path resultsPath(resultsDir);
  if (resultsPath.root_directory().empty()) {
    rDir = m_baseDir / resultsPath;
  }
  else {
    rDir = resultsPath;
  }

  CFinfo << "Results Dir set to: \'" << rDir.string() << "\'\n";
  setDir(m_resultsDir,rDir.string(),true);
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF
