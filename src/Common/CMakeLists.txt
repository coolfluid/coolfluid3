CF_FIND_ORPHAN_FILES()

LIST ( APPEND Common_files
    ArrayAllocator.hpp
    AssertionManager.cpp
    AssertionManager.hpp
    BasicExceptions.cpp
    BasicExceptions.hpp
    BigAllocator.hpp
    BoostIostreams.hpp
    BuilderParser.cpp
    BuilderParser.hpp
    BuilderParserFrameInfo.cpp
    BuilderParserFrameInfo.hpp
    BuilderParserMandType.hpp
    BuilderParserRules.cpp
    BuilderParserRules.hpp
    CF.hpp
    CodeLocation.cpp
    CodeLocation.hpp
    CommonAPI.hpp
    Component.hpp
    Component.cpp
    CPath.hpp
    CPath.cpp
    ConcreteProvider.hpp
    ConfigArgs.cpp
    ConfigArgs.hpp
    ConnectivityTable.hpp
    ConverterTools.cpp
    ConverterTools.hpp
    CoreEnv.hpp
    CoreEnv.cpp
    CoreVars.hpp
    CoreVars.cpp
    DemangledTypeID.cpp
    DemangledTypeID.hpp
    DirPaths.cpp
    DirPaths.hpp
    DynamicObject.hpp
    DynamicObject.cpp
    EnumT.hpp
    EventHandler.hpp
    EventHandler.cpp
    Exception.cpp
    Exception.hpp
    ExportAPI.hpp
    Factory.hpp
    FactoryBase.hpp
    FactoryRegistry.hpp
    FactoryRegistry.cpp
    GeneralStorage.hpp
    HourMinSec.cpp
    HourMinSec.hpp
    LibLoader.cpp
    LibLoader.hpp
    Log.cpp
    Log.hpp
    LogLevel.hpp
    LogLevelFilter.cpp
    LogLevelFilter.hpp
    LogStampFilter.cpp
    LogStampFilter.hpp
    LogStream.cpp
    LogStream.hpp
    LogStringForwarder.hpp
    LogStringForwarder.cpp
    ModuleRegistry.hpp
    ModuleRegisterBase.hpp
    ModuleRegistry.cpp
    ModuleRegister.hpp
    ModuleRegisterBase.cpp
    MemoryAllocator.hpp
    MemoryAllocatorNormal.cpp
    MemoryAllocatorNormal.hpp
    NamedObject.hpp
    NamedObject.cpp
    NonCopyable.hpp
    NonInstantiable.hpp
    ObjectProvider.hpp
    OSystem.cpp
    OSystem.hpp
    OwnedObject.hpp
    PE.cpp
    PE.hpp
    PEInterface.cpp
    PEInterface.hpp
    ProcessInfo.cpp
    ProcessInfo.hpp
    Provider.hpp
    ProviderBase.cpp
    ProviderBase.hpp
    SafePtr.hpp
    SharedPtr.hpp
    SelfRegistPtr.hpp
    SetupObject.hpp
    SetupObject.cpp
    SignalHandler.cpp
    SignalHandler.hpp
    Stopwatch.hpp
    StringOps.hpp
    TimePolicies.cpp
    TimePolicies.hpp
    VarRegistry.hpp
    VarRegistry.cpp
    WorkerStatus.cpp
    WorkerStatus.hpp
    xmlParser.cpp
    xmlParser.h
)

LIST(APPEND Common_libs ${Boost_LIBRARIES})

###############################################################################
# MPI Files
IF(CF_HAVE_MPI)
LIST(APPEND Common_files
      MPI/CommIteratorBase.hpp
      MPI/CommPattern.cpp
      MPI/CommPattern.hpp
      MPI/GlobalReduceMPI.hpp
      MPI/DataType.hpp
      MPI/DataType.cpp
      MPI/DataTypeHandler.cpp
      MPI/DataTypeHandler.hpp
      MPI/DataTypeRegistrar_Helper.cpp
      MPI/DataTypeRegistrar_Helper.hpp
      MPI/DataTypeRegistrar.hpp
      MPI/HelperFuncs.cpp
      MPI/HelperFuncs.hpp
      MPI/MPIInitObject.hpp
      MPI/MPIStructDef.hpp
      MPI/ParVectorMPI.hpp
      MPI/PE_MPI.cpp
      MPI/PE_MPI.hpp
)
ENDIF()

###############################################################################
# MMap allocation
LIST ( APPEND OPTIONAL_dirfiles MemoryAllocatorMMap.hpp MemoryAllocatorMMap.cpp )
IF(CF_HAVE_ALLOC_MMAP AND CF_HAVE_UNISTD_H )
  LIST(APPEND Common_files MemoryAllocatorMMap.hpp MemoryAllocatorMMap.cpp )
ENDIF()

###############################################################################
# Operating System dependent files

### UNIXES ####
IF (UNIX)

  IF ( CF_HAVE_DLOPEN )
    LIST ( APPEND Common_files PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp )
    LIST ( APPEND Common_libs  dl )
    SET ( CF_HAVE_LIBLOADER ON )
  ENDIF()

  IF    ( CF_OS_MACOSX )
    LIST ( APPEND Common_files MacOSX/SignalHandler.hpp MacOSX/SignalHandler.cpp )
    LIST ( APPEND Common_files MacOSX/ProcessInfo.hpp   MacOSX/ProcessInfo.cpp )
  ENDIF ()

  IF    ( CF_OS_LINUX )
    LIST ( APPEND Common_files Linux/ProcessInfo.hpp   Linux/ProcessInfo.cpp )
    LIST ( APPEND Common_files Linux/SignalHandler.hpp Linux/SignalHandler.cpp )
  ENDIF ()

ENDIF(UNIX)

### WINDOWS ####
IF (WIN32)

  LIST ( APPEND Common_files Win32/ProcessInfo.hpp   Win32/ProcessInfo.cpp )
  LIST ( APPEND Common_files Win32/SignalHandler.hpp Win32/SignalHandler.cpp )

  # should search explicitly for dbghelp.dll
  # should search for psapi.dll if not in windows 7 (already in kernel32.dll)

  # libraries for ProcessInfoWin32
  LIST ( APPEND Common_libs dbghelp psapi )

  IF ( CF_HAVE_WINDOWSH )
    LIST ( APPEND Common_files Win32/LibLoader.hpp     Win32/LibLoader.cpp )
    SET ( CF_HAVE_LIBLOADER ON )
  ENDIF()

ENDIF()

IF    ( NOT CF_HAVE_LIBLOADER )
  MESSAGE ( FATAL_ERROR "No dynamic library loader was found!")
ENDIF()

CF_MARK_NOT_ORPHAN (
PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp
MacOSX/SignalHandler.hpp MacOSX/SignalHandler.cpp
MacOSX/ProcessInfo.hpp   MacOSX/ProcessInfo.cpp
Linux/ProcessInfo.hpp    Linux/ProcessInfo.cpp
Linux/SignalHandler.hpp  Linux/SignalHandler.cpp
Win32/ProcessInfo.hpp    Win32/ProcessInfo.cpp
Win32/SignalHandler.hpp  Win32/SignalHandler.cpp
Win32/LibLoader.hpp      Win32/LibLoader.cpp
MemoryAllocatorMMap.hpp  MemoryAllocatorMMap.cpp
)

SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

ADD_SUBDIRECTORY ( MacOSX )   # support for Mac OSX
ADD_SUBDIRECTORY ( Linux  )   # support for Linux
ADD_SUBDIRECTORY ( Win32  )   # support for Win32

ADD_SUBDIRECTORY ( uTests )    # unit tests for Common

