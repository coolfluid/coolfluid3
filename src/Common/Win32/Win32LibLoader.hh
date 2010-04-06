#ifndef Win32LibLoader_hh
#define Win32LibLoader_hh

#include "Common/LibLoader.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Class to load libraries in the Win32 OS
class  Common_API Win32LibLoader :
  public LibLoader {

public: // functions

  /// constructor
  Win32LibLoader();

  /// virtual destructor
  virtual ~Win32LibLoader();

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoaderException if loading fails for any reason
  virtual void load_library(const std::string& lib);

  /// class interface to add paths to search for libraries
  virtual void set_search_paths(std::vector< boost::filesystem::path >& paths);

  private: // data

    /// paths where to search for the libraries to load
    std::vector< boost::filesystem::path > m_search_paths;

}; // end of class Win32LibLoader

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // Win32LibLoader
