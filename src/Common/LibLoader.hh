#ifndef COOLFluiD_Common_LibLoader_hh
#define COOLFluiD_Common_LibLoader_hh

#include <boost/filesystem/path.hpp>

#include "Common/CommonAPI.hh" 

#include "Common/Exception.hh"
#include "Common/NonCopyable.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

class Common_API LibLoaderException : public Common::Exception {
public:

  /// Constructor
  LibLoaderException ( const Common::CodeLocation& where, const std::string& what) :
    Common::Exception(where, what, "LibLoaderException") {}

  /// Copy constructor
  LibLoaderException ( const LibLoaderException& e) throw () : Exception(e) {}

}; // end class LibLoaderException


//////////////////////////////////////////////////////////////////////////////

/// Class that explicit loads shared libraries
/// independently of the operating system
/// @author Tiago Quintino
class Common_API LibLoader : public Common::NonCopyable<LibLoader> {

public: // methods

  /// constructor
  LibLoader();

  /// virtual destructor
  virtual ~LibLoader();

  /// class interface to load a library depending on the operating system
  /// and the library loading algorithm
  /// @throw LibLoaderException if loading fails for any reason
  ///
  virtual void load_library(const std::string& lib) = 0;

  /// class interface to add paths to search for libraries
  ///
  virtual void set_search_paths(std::vector< boost::filesystem::path >& paths) = 0;

  /// Gets the Class name
  static std::string getClassName() { return "LibLoader"; }

}; // class FileHandlerOutput

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Common_LibLoader_hh
