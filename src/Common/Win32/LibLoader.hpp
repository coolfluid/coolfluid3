#ifndef Win32LibLoader_hpp
#define Win32LibLoader_hpp

#include "Common/LibLoader.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace Win32 {

////////////////////////////////////////////////////////////////////////////////

/// Class to load libraries in the Win32 OS
class  Common_API LibLoader : public Common::LibLoader {

public: // functions

  /// constructor
  LibLoader();

  /// virtual destructor
  virtual ~LibLoader();

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoader if loading fails for any reason
  virtual void load_library(const std::string& lib);

  /// class interface to add paths to search for libraries
  virtual void set_search_paths(std::vector< boost::filesystem::path >& paths);

  private: // data

  /// paths where to search for the libraries to load
  std::vector< boost::filesystem::path > m_search_paths;

}; // end of class Win32LibLoader

////////////////////////////////////////////////////////////////////////////////

} // Win32
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // Win32LibLoader
