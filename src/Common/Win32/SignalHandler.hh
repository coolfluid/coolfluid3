#ifndef CF_Common_Win32_SignalHandler_hh
#define CF_Common_Win32_SignalHandler_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/SignalHandler.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
	namespace Win32 {

//////////////////////////////////////////////////////////////////////////////

/// This class handles of signals from the Win32 operating system
/// @author Tiago Quintino
class Common_API SignalHandlerWin32 : public Common::SignalHandler {

public: // methods

  /// Constructor
  SignalHandlerWin32();

  /// Default destructor
  virtual ~SignalHandlerWin32();

  /// Regists the signal handlers that will be handled by this class
  virtual void registSignalHandlers();

}; // end of class SignalHandlerWin32

//////////////////////////////////////////////////////////////////////////////

} // Win32
} // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_Win32_SignalHandler_hh
