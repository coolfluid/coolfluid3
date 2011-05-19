// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Log_hpp
#define CF_Common_Log_hpp

#include "Common/CommonAPI.hpp"
#include "Common/LogLevel.hpp"
#include "Common/LogStream.hpp"
#include "Common/CodeLocation.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

class LogStream;

/// @brief Main class of the logging system.

/// This class manages all streams used for logs. This class is a singleton. @n
/// @c Logger manages 5 streams: @e info, @e error, @e warning, @e debug and
/// @e trace. These streams differ on the type of messages they forward. By
/// default, only streams @c #ERROR, @c #WARNING, @c #INFO will be outputted.
/// This can be changed using Logger::set_log_level().
/// By default no file is open.
/// Once an MPI environment has been initialized, @c #openFiles() can be called.
/// Two files descriptors are created: one for a @e log file and another one
/// for a @e trace file; respectively <code>output-p<i>i</i>.log</code> and
/// <code>output-p<i>i</i>.trace</code>, where <code><i>i</i></code> if the
/// MPI rank number).

/// @see LogStream
/// @author Quentin Gasper
class Common_API Logger : public boost::noncopyable
{

  public:

//  /// @brief Stream types
//  enum StreamType
//  {
//    /// @brief Stream for error messages.
//    ERROR=1,
//    /// @brief Stream for warning messages.
//    WARNING=2,
//    /// @brief Stream for normal messages.
//    INFO=3,
//    /// @brief Stream for debug messages.
//    DEBUG=4,
//    /// @brief Stream for trace message.
//    TRACE=5
//  };

  /// @brief Gives the current @c #Logger instance.

  /// If no instance exists, a new one is created.
  /// @return Returns the current instance.
  static Logger& instance();

  /// @returns the class name
  static std::string type_name() { return  "Logger"; }

  /// @brief Gives the info stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the info stream.
  LogStream & Info (const CodeLocation & place);

  /// @brief Gives the error stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the error stream.
  LogStream & Error (const CodeLocation & place);

  /// @brief Gives the warning stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the warning stream.
  LogStream & Warn (const CodeLocation & place);

  /// @brief Gives the debug stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the debug stream.
  LogStream & Debug (const CodeLocation & place);

  /// @brief Gives the trace stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the trace stream.
  LogStream & Trace (const CodeLocation & place);

  LogStream & getStream(LogLevel type);

  /// @brief Creates file descriptors and gives them to streams.
  void openFiles();

  void set_log_level(const Uint log_level);

  private :

  /// @brief Managed streams.

  /// The key is the stream type. The value is a pointer to the stream.
  std::map<LogLevel, LogStream *> m_streams;

  /// @brief Constructor
  Logger();

  /// @brief Destructor
  ~Logger();

}; // class Logger

////////////////////////////////////////////////////////////////////////////////
// Logging macros
////////////////////////////////////////////////////////////////////////////////

/// these are always defined

#define CFinfo      CF::Common::Logger::instance().Info (FromHere())
#define CFerror     CF::Common::Logger::instance().Error(FromHere())
#define CFwarn      CF::Common::Logger::instance().Warn (FromHere())
#define CFdebug_always  CF::Common::Logger::instance().Debug(FromHere())
#define CFtrace_always  CF::Common::Logger::instance().Trace(FromHere())
#define CFflush     CF::Common::LogStream::ENDLINE
#define CFendl  '\n' << CFflush

#ifdef CF_NO_DEBUG_LOG
  #define CFdebug if(0) CFdebug_always
#else
  #define CFdebug CFdebug_always
#endif
#ifdef CF_NO_TRACE
  #define CFtrace if(0) CFtrace_always
#else
  #define CFtrace CFtrace_always
#endif


////////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_LOG
#define CFLog(n,x) CFinfo << n << x << CFflush;
#define CFLogVar(x) CFinfo << #x << " = " << x << CFendl;
#else
#define CFLog(n,x)
#define CFLogVar(x)
#endif

////////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_LOG

#define CFLogDebug(x)        CFdebug << x << CFflush;
#define CFLogDebugVerbose(x) CFdebug << VERBOSE << x << CFflush;

#else // CF_NO_DEBUG_LOG

#define CFLogDebug(x)
#define CFLogDebugVerbose(x)

#endif // CF_NO_DEBUG_LOG




#define CFLogInfo(x)   CFinfo  << x << CFflush;
#define CFLogWarn(x)   CFwarn  << x << CFflush;
#define CFLogError(x)  CFerror << x << CFflush;

////////////////////////////////////////////////////////////////////////////////
// Tracing macros
////////////////////////////////////////////////////////////////////////////////

// #include "LogStream.hpp"

/// Class to help trace functions which is easy to use and catches all function exits (return,throw,...)
/// @author Dries Kimpe
class Common_API AutoTracer {
  public:
    /// constructor
  AutoTracer (const char * Function, const char * File, int Line) : m_Function(Function), m_File(File), m_Line(Line)
  {
    CFtrace << "### BEGIN ### " << m_Function << " : " << m_File << " : " << m_Line << "\n" << CFflush; /*CFtrace.flush();*/
  }

    /// destructor
  ~AutoTracer()
  {
    CFtrace << "### END   ### " << m_Function << " : " << m_File << " : " << m_Line << "\n" << CFflush; /*CFtrace.flush();*/
  }

  protected: // data
    const char * m_Function;
    const char * m_File;
    int m_Line;
};

////////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_TRACE

#define CFAUTOTRACE_PASTE_(a,b) a##b
#define CFAUTOTRACE_PASTE(a,b) CFAUTOTRACE_PASTE_(a,b)
#define CFAUTOTRACE ::CF::Common::AutoTracer CFAUTOTRACE_PASTE(AutoTrace_Uniq,__LINE__) (__FUNCTION__,__FILE__,__LINE__)

#define CFTRACEBEGIN CFtrace << "### BEGIN ### " << __FUNCTION__ << " : " << __FILE__ << " : " << __LINE__ << "\n" << CFflush; //CFtrace.flush();
#define CFTRACEEND   CFtrace << "### END ##### " << __FUNCTION__ << " : " << __FILE__ << " : " << __LINE__ << "\n" << CFflush;/* CFtrace.flush();*/

#else // CF_NO_TRACE

#define CFAUTOTRACE

#define CFTRACEBEGIN
#define CFTRACEEND

#endif // CF_NO_TRACE

////////////////////////////////////////////////////////////////////////////////
// Debugging macros
////////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_MACROS

/// Definition of a macro for placing a debug point in the code
#define CF_DEBUG_POINT  CFdebug << "DEBUG : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for outputing an object that implements the output stream operator
#define CF_DEBUG_OBJ(x) CFdebug << "DEBUG : OBJECT " << #x << " -> " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for outputing a debug string in the code
#define CF_DEBUG_STR(x) CFdebug << "DEBUG : STRING : " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush()
/// Definition of a macro for debug abort
#define CF_DEBUG_ABORT  CFdebug << "DEBUG : ABORT " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFdebug.flush() ; abort()

#else // CF_NO_DEBUG_MACROS

#define CF_DEBUG_POINT
#define CF_DEBUG_OBJ(x)
#define CF_DEBUG_STR(x)
#define CF_DEBUG_ABORT

#endif // CF_NO_DEBUG_MACROS

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Log.hpp
