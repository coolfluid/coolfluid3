#ifndef CF_Environment_DirPaths_hpp
#define CF_Environment_DirPaths_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Common/BasicExceptions.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a singleton object where
/// the paths for certain directories are stored.
/// This class is a Singleton pattern implementation.
/// @author Tiago Quintino
class Common_API DirPaths : public boost::noncopyable {

public: // methods

  /// Set the Base dir path
  /// @throw Filesystem if dir does not exist
  void setBaseDir(const std::string& baseDir);

  /// Set the repository
  void setRepositoryURL(const std::string& url);

  /// Set the Modules dir paths
  /// @throw Filesystem if one dir does not exist
  void addModuleDirs(const std::vector<std::string>& modulesDir);

  /// Set the Working dir path
  /// @throw Filesystem if dir does not exist
  void setWorkingDir(const std::string& workingDir);

  /// Set the Results dir path
  /// @throw Filesystem if dir does not exist
  void setResultsDir(const std::string& resultsDir);

  /// Get the repository URL
  std::string getRepositoryURL() const { return m_repURL; }

  /// Get the Working dir path
  boost::filesystem::path getWorkingDir() const { return m_workinDir; }

  /// Get the Results dir path
  boost::filesystem::path getResultsDir() const {  return m_resultsDir; }

  /// Get the Base dir path
  boost::filesystem::path getBaseDir() const {  return m_baseDir; }

  /// Get the Modules dir paths.
  std::vector< boost::filesystem::path > getModulesDir() const { return m_modulesDir; }

public: // methods for singleton

  /// @return the instance of this singleton
  static DirPaths& instance();

protected: // methods

  /// Sets a dir path
  /// @param dpath the path to be set (usually a private variable of the class)
  /// @param dstr the string to convert to the path
  /// @param createdir if directory does not exist, create it
  /// @throw Filesystem if dir does not exist, and createdir is false
  void setDir(boost::filesystem::path& dpath, const std::string& dstr, const bool createdir=false);

  /// Throw exception for all the supplied paths
  void checkThrowMultipleBadDir(const std::vector< boost::filesystem::path >& paths);

private: // methods

  /// Default constructor
  DirPaths();

  /// Default destructor
  ~DirPaths();

private: // data

  /// base dir path
  boost::filesystem::path m_baseDir;
  /// the modules directories paths where to search for module libs
  std::vector< boost::filesystem::path > m_modulesDir;
  /// the working dir path
  boost::filesystem::path m_workinDir;
  /// the results dir path
  boost::filesystem::path m_resultsDir;
  /// the url for file repository
  std::string m_repURL;

}; // end of class DirPaths

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Environment_DirPaths_hpp
