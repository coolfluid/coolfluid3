#ifndef CF_Common_OSystem_hpp
#define CF_Common_OSystem_hpp

#include "Common/Exception.hpp"
#include "Common/SafePtr.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

  class ProcessInfo;
  class SignalHandler;
  class LibLoader;

////////////////////////////////////////////////////////////////////////////////

class Common_API OSystemError : public Common::Exception {

public:

  /// Constructor
  OSystemError ( const Common::CodeLocation& where, const std::string& what);
}; // end class OSystem

////////////////////////////////////////////////////////////////////////////////

/// Represents the operating system
/// @author Tiago Quintino
class Common_API OSystem : public boost::noncopyable {

public: // methods

  /// @return the single object that represents the operating system
  static OSystem& getInstance();

  /// @return ProcessInfo object
  Common::SafePtr<Common::ProcessInfo> getProcessInfo();

  /// @return SignalHandler object
  Common::SafePtr<Common::SignalHandler> getSignalHandler();

  /// @return LibLoader object
  Common::SafePtr<Common::LibLoader> getLibLoader();

  /// Executes the command passed in the string
  /// @todo should return the output of the command but not yet implemented.
  void executeCommand (const std::string& call);

private: // functions

  /// constructor
  OSystem ();
  /// destructor
  ~OSystem ();

private: // data

  /// memory usage object
  Common::ProcessInfo * m_process_info;
  /// signal handler object
  Common::SignalHandler * m_sig_handler;
  /// libloader object
  Common::LibLoader * m_lib_loader;

}; // class FileHandlerOutput

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OSystem_hpp
