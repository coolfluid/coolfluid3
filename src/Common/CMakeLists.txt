LIST ( APPEND Common_files 
   Log.cxx
   Log.hh
   LogLevel.hh
   LogLevelFilter.cxx
   LogLevelFilter.hh
   LogStampFilter.cxx
   LogStampFilter.hh
   LogStream.cxx
   LogStream.hh
   LogStringForwarder.hh
   CodeLocation.cxx
   CodeLocation.hh
   Common.hh
   ExportAPI.hh
   NonCopyable.hh
   NonInstantiable.hh
   PE.cxx
   PE.hh
   PEInterface.cxx
   PEInterface.hh
   StringOps.hh
   StringOps.cxx
   WorkerStatus.cxx
   WorkerStatus.hh
   AssertionManager.cxx
   BasicExpections.cxx
   DemangledTypeID.cxx
   Exception.cxx
   OSystem.cxx
AssertionManager.hh
BasicExpections.hh
CF.hh
DemangledTypeID.hh
EnumT.hh
Exception.hh
Linux/ProcessInfo.cxx
Linux/SignalHandler.cxx
Linux/ProcessInfo.hh
Linux/SignalHandler.hh
MacOSX/ProcessInfo.hh
MacOSX/SignalHandler.hh
MacOSX/ProcessInfo.cxx
MacOSX/SignalHandler.cxx
OSystem.hh
PtrAlloc.hh )

###############################################################################
# MPI Files
IF(CF_HAVE_MPI)
 LIST(APPEND Common_files
      MPI/CommIteratorBase.hh
      MPI/CommPattern.cxx
      MPI/CommPattern.hh
      MPI/GlobalReduceMPI.hh
      MPI/MPIDataType.cxx
      MPI/MPIDataTypeHandler.cxx
      MPI/MPIDataTypeHandler.hh
      MPI/MPIDataType.hh
      MPI/MPIDataTypeRegistrar_Helper.cxx
      MPI/MPIDataTypeRegistrar_Helper.hh
      MPI/MPIDataTypeRegistrar.hh
      MPI/MPIException.hh
      MPI/MPIHelper.cxx
      MPI/MPIHelper.hh
      MPI/MPIInitObject.hh
      MPI/MPIStructDef.hh
      MPI/ParVectorException.hh
      MPI/ParVectorMPI.hh
      MPI/PatternFullExchange.hh
      MPI/PatternRing.hh
      MPI/PEInterfaceMPI.cxx
      MPI/PEInterfaceMPI.hh
 )
ENDIF()

###############################################################################
# Operating System dependent files

### UNIXES ####
IF (UNIX)

  IF ( CF_HAVE_DLOPEN )
    LIST ( APPEND Common_files PosixDlopenLibLoader.hh PosixDlopenLibLoader.cxx )
    LIST ( APPEND Common_libs  dl )
    SET ( CF_HAVE_LIBLOADER ON )
  ENDIF()

  IF    ( CF_OS_MACOSX )
    LIST ( APPEND Common_files MacOSX/SignalHandler.hh MacOSX/SignalHandler.cxx )
    LIST ( APPEND Common_files MacOSX/ProcessInfo.hh   MacOSX/ProcessInfo.cxx )
  ENDIF ()

  IF    ( CF_OS_LINUX )
    LIST ( APPEND Common_files Linux/ProcessInfo.hh   Linux/ProcessInfo.cxx )
    LIST ( APPEND Common_files Linux/SignalHandler.hh Linux/SignalHandler.cxx )
  ENDIF ()

ENDIF(UNIX)

### WINDOWS ####
IF (WIN32)
  
  LIST ( APPEND Common_files Win32/ProcessInfo.hh   Win32/ProcessInfo.cxx )
  LIST ( APPEND Common_files Win32/SignalHandler.hh Win32/SignalHandler.cxx )

  # should search explicitly for dbghelp.dll
  # should search for psapi.dll if not in windows 7 (already in kernel32.dll)

  # libraries for ProcessInfoWin32
  LIST ( APPEND Common_libs dbghelp psapi )

  IF ( CF_HAVE_WINDOWSH )
    LIST ( APPEND Common_files Win32/LibLoader.hh     Win32/LibLoader.cxx )
    SET ( CF_HAVE_LIBLOADER ON )
  ENDIF()

ENDIF()

IF    ( NOT CF_HAVE_LIBLOADER )
  MESSAGE ( FATAL_ERROR "No dynamic library loader was found!")
ENDIF()



SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

ADD_SUBDIRECTORY ( MacOSX )   # support for Mac OSX
ADD_SUBDIRECTORY ( Linux  )   # support for Linux
ADD_SUBDIRECTORY ( Win32  )   # support for Win32

ADD_SUBDIRECTORY(tests)

