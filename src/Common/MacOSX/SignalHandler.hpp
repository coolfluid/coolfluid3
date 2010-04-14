#ifndef CF_Common_MacOSX_SignalHandler_hh
#define CF_Common_MacOSX_SignalHandler_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"

#include "Common/SignalHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace MacOSX {

////////////////////////////////////////////////////////////////////////////////

/// This class handles of signals from the MacOSX operating system
/// @author Tiago Quintino
class Common_API SignalHandler : public Common::SignalHandler {

public: // methods

  /// Constructor
  SignalHandler();

  /// Default destructor
  virtual ~SignalHandler();

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

protected:

  /// SIGFPE signal handler
  static int handleSIGFPE(int signal);

  /// SIGSEGV signal handler
  static int handleSIGSEGV(int signal);

}; // SignalHandler

////////////////////////////////////////////////////////////////////////////////

    } // MacOSX
  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MacOSX_SignalHandler_hh
