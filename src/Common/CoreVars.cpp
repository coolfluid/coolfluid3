#include "Common/CoreVars.hpp"
#include "Common/ModuleRegisterBase.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

CoreVars::CoreVars() :
  OnlyCPU0Writes       ( true  ),
  RegistSignalHandlers ( true  ),
  TraceActive          ( false ),
  TraceToStdOut        ( false ),
  VerboseEvents        ( false ),
  ErrorOnUnusedConfig  ( false ),
  MainLoggerFileName("output.log"),
  ExceptionLogLevel( (Uint) VERBOSE),
  InitArgs()
{
  InitArgs.first  = 0;
  InitArgs.second = CFNULL;
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF
