#ifndef COOLFluiD_Common_PosixDlopenLibLoader_hh
#define COOLFluiD_Common_PosixDlopenLibLoader_hh

#include "Common/LibLoader.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

class Common_API PosixDlopenLibLoader : public LibLoader {

public: // functions

  /// constructor
  PosixDlopenLibLoader();

  /// virtual destructor
  virtual ~PosixDlopenLibLoader();

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoaderException if loading fails for any reason
  ///
  virtual void load_library(const std::string& lib);

  /// class interface to add paths to search for libraries
  ///
  virtual void set_search_paths(std::vector< boost::filesystem::path >& paths);

  private: // data

    /// paths where to search for the libraries to load
    std::vector< boost::filesystem::path > m_search_paths;

}; // end of class PosixDlopenLibLoader

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Common_PosixDlopenLibLoader_hh
