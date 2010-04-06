#ifndef COOLFluiD_Common_CFLog_hh
#define COOLFluiD_Common_CFLog_hh

#include "Common/Common.hh"
#include "CFLogLevel.hh"
#include "CFLogStream.hh"
#include "CodeLocation.hh"

#include <map>

/////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Common {
 
 class CFLogStream;

 /// @brief Main class of the logging system.
 
 /// This class manages all streams used for logs. This class is a singleton.@n
 /// @c CFLogger manages 5 streams: @e info, @e error, @e warning, @e debug and 
 /// @e trace. These streams differ on the type of messages they forward. By 
 /// default, streams default level is @c #NORMAL and no file is open. Once an 
 /// MPI environment has been initialized, @c #openFiles() can be called. 
 /// Two files descriptors are created: one for a @e log file and another one
 /// for a @e trace file; respectively <code>output-p<i>i</i>.log</code> and 
 /// <code>output-p<i>i</i>.trace</code>, where <code><i>i</i></code> if the 
 /// MPI rank number).
 
 /// @see CFLogStream
 /// @author Quentin Gasper 
 class Common_API CFLogger /*: public Common::NonCopyable<CFLogger>*/
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
   
   /// @brief Gives the current @c #CFLogger instance.
   
   /// If no instance exists, a new one is created.
   /// @return Returns the current instance.
   static CFLogger & getInstance();
   
   /// @brief Gives the info stream.
   
   /// @param place The code location from where this method was called
   /// @return Returns a reference to the info stream.
   CFLogStream & getInfo (const CodeLocation & place = CodeLocation("", 0, ""));
   
   /// @brief Gives the error stream.
   
   /// @param place The code location from where this method was called
   /// @return Returns a reference to the error stream.
   CFLogStream & getError (const CodeLocation & place = CodeLocation("", 0, ""));
   
   /// @brief Gives the warning stream.
   
   /// @param place The code location from where this method was called
   /// @return Returns a reference to the warning stream.
   CFLogStream & getWarn (const CodeLocation & place = CodeLocation("", 0, ""));
   
   /// @brief Gives the debug stream.
   
   /// @param place The code location from where this method was called
   /// @return Returns a reference to the debug stream.
   CFLogStream & getDebug (const CodeLocation & place = CodeLocation("", 0, ""));
   
   /// @brief Gives the trace stream.
   
   /// @param place The code location from where this method was called
   /// @return Returns a reference to the trace stream.
   CFLogStream & getTrace (const CodeLocation & place = CodeLocation("", 0, ""));
   
   /// @brief Creates file descriptors and gives them to streams.
   void openFiles();
   
  private :
   
   /// @brief Managed streams.
   
   /// The key is the stream type. The value is a pointer to the stream.
   std::map<StreamType, CFLogStream *> m_streams; 
   
   /// @brief Constructor
   CFLogger();
   
   /// @brief Destructor
   ~CFLogger();
   
 }; // class CFLogger
 
//////////////////////////////////////////////////////////////////////////////
// Logging macros
//////////////////////////////////////////////////////////////////////////////

/// these are always defined

#define CFinfo   CFLogger::getInstance().getInfo (FromHere())
#define CFerr    CFLogger::getInstance().getError(FromHere())
#define CFwarn   CFLogger::getInstance().getWarn (FromHere())
#define CFdebug  CFLogger::getInstance().getDebug(FromHere())
#define CFtrace  CFLogger::getInstance().getTrace(FromHere())
#define CFendl   CFLogStream::ENDLINE

 //////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_LOG
#define CFLog(n,x) CFinfo << n << x << CFendl;
#else
#define CFLog(n,x)
#endif

 //////////////////////////////////////////////////////////////////////////////

#ifndef CF_NO_DEBUG_LOG

#define CFLogDebug(x)    CFdebug << x << CFendl;
#define CFLogDebugMax(x) CFdebug << VERBOSE << x << CFendl;
#define CFLogDebugMed(x) CFdebug << VERBOSE << x << CFendl;
#define CFLogDebugMin(x) CFdebug << x << CFendl;

#else // CF_NO_DEBUG_LOG

#define CFLogDebug(x)
#define CFLogDebugMax(x)
#define CFLogDebugMed(x)
#define CFLogDebugMin(x)

#endif // CF_NO_DEBUG_LOG

#define CFLogInfo(x)   CFinfo << x << CFendl;
#define CFLogNotice(x) CFinfo << x << CFendl;
#define CFLogWarn(x)   CFwarn << x << CFendl;
#define CFLogError(x)  CFerr << x << CFendl; 

 //////////////////////////////////////////////////////////////////////////////
// Tracing macros
 //////////////////////////////////////////////////////////////////////////////
 
// #include "CFLogStream.hh"
 
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
#define CFAUTOTRACE /*::COOLFluiD::Common::*/AutoTracer CFAUTOTRACE_PASTE(AutoTrace_Uniq,__LINE__) (__FUNCTION__,__FILE__,__LINE__)

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

////////////////////////////////////////////////////////////////////////////
 
} // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_Common_CFLog.hh
