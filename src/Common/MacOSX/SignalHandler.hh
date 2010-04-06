#ifndef COOLFluiD_Common_SignalHandlerMacOSX_hh
#define COOLFluiD_Common_SignalHandlerMacOSX_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/SignalHandler.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class handles of signals from the MacOSX operating system
/// @author Tiago Quintino
class Common_API SignalHandlerMacOSX : public SignalHandler {

public: // methods

  /// Constructor
  SignalHandlerMacOSX();

  /// Default destructor
  virtual ~SignalHandlerMacOSX();

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

protected:
  
  /// SIGFPE signal handler
  static int handleSIGFPE(int signal);

  /// SIGSEGV signal handler
  static int handleSIGSEGV(int signal);

}; // end of class SignalHandlerMacOSX

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_SignalHandlerMacOSX_hh
