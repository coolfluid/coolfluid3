#ifndef CF_Common_Linux_SignalHandler_hh
#define CF_Common_Linux_SignalHandler_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/SignalHandler.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace Linux {

//////////////////////////////////////////////////////////////////////////////

/// This class handles of signals from the Linux operating system
/// @author Tiago Quintino
class Common_API SignalHandler : public Common::SignalHandler {

public: // methods

  /// Constructor
  SignalHandler();

  /// Default destructor
  virtual ~SignalHandler();

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

protected: // methods

  /// SIGFPE signal handler
  static int handleSIGFPE(int signal);

  /// SIGSEGV signal handler
  static int handleSIGSEGV(int signal);

}; // end of class SignalHandler

//////////////////////////////////////////////////////////////////////////////

    } // Linux
  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_Linux_SignalHandler_hh
