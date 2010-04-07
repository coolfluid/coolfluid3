#ifndef CF_Common_Log_hh
#define CF_Common_Log_hh

#include "Common/CommonAPI.hh"
#include "Common/LogLevel.hh"
#include "Common/LogStream.hh"
#include "Common/CodeLocation.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common {

class LogStream;

/// @brief Main class of the logging system.

/// This class manages all streams used for logs. This class is a singleton. @n
/// @c Logger manages 5 streams: @e info, @e error, @e warning, @e debug and
/// @e trace. These streams differ on the type of messages they forward. By
/// default, streams default level is @c #NORMAL and no file is open. Once an
/// MPI environment has been initialized, @c #openFiles() can be called.
/// Two files descriptors are created: one for a @e log file and another one
/// for a @e trace file; respectively <code>output-p<i>i</i>.log</code> and
/// <code>output-p<i>i</i>.trace</code>, where <code><i>i</i></code> if the
/// MPI rank number).

/// @see LogStream
/// @author Quentin Gasper
class Common_API Logger : public Common::NonCopyable<Logger>
{

  public:

  /// @brief Stream types
  enum StreamType
  {
    /// @brief Stream for normal messages.
    INFO_STREAM,

    /// @brief Stream for error messages.
    ERROR_STREAM,

    /// @brief Stream for warning messages.
    WARN_STREAM,

    /// @brief Stream for debug messages.
    DEBUG_STREAM,

    /// @brief Stream for trace message.
    TRACE_STREAM
  };

  /// @brief Gives the current @c #Logger instance.

  /// If no instance exists, a new one is created.
  /// @return Returns the current instance.
  static Logger & getInstance();

  /// @brief Gives the info stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the info stream.
  LogStream & getInfo (const CodeLocation & place = CodeLocation("", 0, ""));

  /// @brief Gives the error stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the error stream.
  LogStream & getError (const CodeLocation & place = CodeLocation("", 0, ""));

  /// @brief Gives the warning stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the warning stream.
  LogStream & getWarn (const CodeLocation & place = CodeLocation("", 0, ""));

  /// @brief Gives the debug stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the debug stream.
  LogStream & getDebug (const CodeLocation & place = CodeLocation("", 0, ""));

  /// @brief Gives the trace stream.

  /// @param place The code location from where this method was called
  /// @return Returns a reference to the trace stream.
  LogStream & getTrace (const CodeLocation & place = CodeLocation("", 0, ""));

  /// @brief Creates file descriptors and gives them to streams.
  void openFiles();

  private :

  /// @brief Managed streams.

  /// The key is the stream type. The value is a pointer to the stream.
  std::map<StreamType, LogStream *> m_streams;

  /// @brief Constructor
  Logger();

  /// @brief Destructor
  ~Logger();

}; // class Logger

//////////////////////////////////////////////////////////////////////////////
// Logging macros
//////////////////////////////////////////////////////////////////////////////

/// these are always defined

#define CFinfo   Logger::getInstance().getInfo (FromHere())
#define CFerr    Logger::getInstance().getError(FromHere())
#define CFwarn   Logger::getInstance().getWarn (FromHere())
#define CFdebug  Logger::getInstance().getDebug(FromHere())
#define CFtrace  Logger::getInstance().getTrace(FromHere())
#define CFendl   LogStream::ENDLINE

//////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_LOG
#define CFLog(n,x) CFinfo << n << x << CFendl;
#else
#define CFLog(n,x)
#endif

//////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_LOG

#define CFLogDebug(x)        CFdebug << x << CFendl;
#define CFLogDebugVerbose(x) CFdebug << VERBOSE << x << CFendl;

#else // CF_NO_DEBUG_LOG

#define CFLogDebug(x)
#define CFLogDebugVerbose(x)

#endif // CF_NO_DEBUG_LOG

#define CFLogInfo(x)   CFinfo << x << CFendl;
#define CFLogWarn(x)   CFwarn << x << CFendl;
#define CFLogError(x)  CFerr << x << CFendl;

//////////////////////////////////////////////////////////////////////////////
// Tracing macros
//////////////////////////////////////////////////////////////////////////////

// #include "LogStream.hh"

/// Class to help trace functions which is easy to use and catches all function exits (return,throw,...)
/// @author Dries Kimpe
class Common_API AutoTracer {
  public:
    /// constructor
  AutoTracer (const char * Function, const char * File, int Line) : m_Function(Function), m_File(File), m_Line(Line)
  {
    CFtrace << "### BEGIN ### " << m_Function << " : " << m_File << " : " << m_Line << "\n" << CFendl; /*CFtrace.flush();*/
  }

    /// destructor
  ~AutoTracer()
  {
    CFtrace << "### END   ### " << m_Function << " : " << m_File << " : " << m_Line << "\n" << CFendl; /*CFtrace.flush();*/
  }

  protected: // data
    const char * m_Function;
    const char * m_File;
    int m_Line;
};

//////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_TRACE

#define CFAUTOTRACE_PASTE_(a,b) a##b
#define CFAUTOTRACE_PASTE(a,b) CFAUTOTRACE_PASTE_(a,b)
#define CFAUTOTRACE /*::CF::Common::*/AutoTracer CFAUTOTRACE_PASTE(AutoTrace_Uniq,__LINE__) (__FUNCTION__,__FILE__,__LINE__)

#define CFTRACEBEGIN CFtrace << "### BEGIN ### " << __FUNCTION__ << " : " << __FILE__ << " : " << __LINE__ << "\n" << CFendl; //CFtrace.flush();
#define CFTRACEEND   CFtrace << "### END ##### " << __FUNCTION__ << " : " << __FILE__ << " : " << __LINE__ << "\n" << CFendl;/* CFtrace.flush();*/

#else // CF_NO_TRACE

#define CFAUTOTRACE

#define CFTRACEBEGIN
#define CFTRACEEND

#endif // CF_NO_TRACE

//////////////////////////////////////////////////////////////////////////////
// Debugging macros
//////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_MACROS

/// Definition of a macro for placing a debug point in the code
#define CF_DEBUG_POINT  CFerr << "DEBUG : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFerr.flush()
/// Definition of a macro for outputing an object that implements the output stream operator
#define CF_DEBUG_OBJ(x) CFerr << "DEBUG : OBJECT " << #x << " -> " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFerr.flush()
/// Definition of a macro for outputing a debug string in the code
#define CF_DEBUG_STR(x) CFerr << "DEBUG : STRING : " << x << " : " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFerr.flush()
/// Definition of a macro for debug exit
#define CF_DEBUG_EXIT   CFerr << "DEBUG : EXIT "  << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFerr.flush() ; exit(0)
/// Definition of a macro for debug abort
#define CF_DEBUG_ABORT  CFerr << "DEBUG : ABORT " << __FILE__ << " : " << __LINE__ << " : " << __FUNCTION__ << "\n" ; CFerr.flush() ; abort()

#else // CF_NO_DEBUG_MACROS

#define CF_DEBUG_POINT
#define CF_DEBUG_OBJ(x)
#define CF_DEBUG_STR(x)
#define CF_DEBUG_EXIT
#define CF_DEBUG_ABORT

#endif // CF_NO_DEBUG_MACROS

//////////////////////////////////////////////////////////////////////////////

} // namespace Common

} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Log.hh
