LIST ( APPEND Common_files 
     AssertionManager.cxx
     AssertionManager.hh
     BasicExceptions.cxx
     BasicExceptions.hh
     BoostIostreams.hh
     CF.hh
     CodeLocation.cxx
     CodeLocation.hh
     CommonAPI.hh
     DemangledTypeID.cxx
     DemangledTypeID.hh
     EnumT.hh
     Exception.cxx
     Exception.hh
     ExportAPI.hh
     LibLoader.cxx
     LibLoader.hh
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
     LogStringForwarder.cxx
     NonCopyable.hh
     NonInstantiable.hh
     OSystem.cxx
     OSystem.hh
     OwnedObject.hh
     PE.cxx
     PE.hh
     FakePE.cxx
     FakePE.hh
     PEInterface.cxx
     PEInterface.hh
     ProcessInfo.cxx
     ProcessInfo.hh
     ProviderBase.cxx
     ProviderBase.hh
     PtrAlloc.hh
     SafePtr.hh
     SelfRegistPtr.hh
     SignalHandler.cxx
     SignalHandler.hh
     StringOps.cxx
     StringOps.hh
     WorkerStatus.cxx
     WorkerStatus.hh
)

LIST(APPEND Common_libs ${Boost_LIBRARIES})

###############################################################################
# MPI Files
IF(CF_HAVE_MPI)
 LIST(APPEND Common_files
      MPI/CommIteratorBase.hh
      MPI/CommPattern.cxx
      MPI/CommPattern.hh
      MPI/GlobalReduceMPI.hh
      MPI/DataType.hh
      MPI/DataType.cxx
      MPI/DataTypeHandler.cxx
      MPI/DataTypeHandler.hh
      MPI/DataTypeRegistrar_Helper.cxx
      MPI/DataTypeRegistrar_Helper.hh
      MPI/DataTypeRegistrar.hh
      MPI/HelperFuncs.cxx
      MPI/HelperFuncs.hh
      MPI/MPIInitObject.hh
      MPI/MPIStructDef.hh
      MPI/ParVectorMPI.hh
      MPI/PE_MPI.cxx
      MPI/PE_MPI.hh
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

CF_MARK_NOT_ORPHAN ( 
	PosixDlopenLibLoader.hh PosixDlopenLibLoader.cxx
	MacOSX/SignalHandler.hh MacOSX/SignalHandler.cxx
	MacOSX/ProcessInfo.hh   MacOSX/ProcessInfo.cxx
	Linux/ProcessInfo.hh    Linux/ProcessInfo.cxx
	Linux/SignalHandler.hh  Linux/SignalHandler.cxx
	Win32/ProcessInfo.hh    Win32/ProcessInfo.cxx
	Win32/SignalHandler.hh  Win32/SignalHandler.cxx
	Win32/LibLoader.hh      Win32/LibLoader.cxx
)

SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

ADD_SUBDIRECTORY ( MacOSX )   # support for Mac OSX
ADD_SUBDIRECTORY ( Linux  )   # support for Linux
ADD_SUBDIRECTORY ( Win32  )   # support for Win32

ADD_SUBDIRECTORY ( tests )    # unit tests for Common

