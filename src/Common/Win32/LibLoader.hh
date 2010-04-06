#ifndef COOLFluiD_Common_SignalHandlerWin32_hh
#define COOLFluiD_Common_SignalHandlerWin32_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/SignalHandler.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class handles of signals from the Win32 operating system
/// @author Tiago Quintino
class Common_API SignalHandlerWin32 : public SignalHandler {

public: // methods

  /// Constructor
  SignalHandlerWin32();

  /// Default destructor
  virtual ~SignalHandlerWin32();

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

}; // end of class SignalHandlerWin32

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_SignalHandlerWin32_hh
