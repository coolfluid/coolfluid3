coolfluid_find_orphan_files()

list( APPEND coolfluid_common_files
    AllocatedComponent.hpp
    AllocatedComponent.cpp
    Assertions.cpp
    Assertions.hpp
    BasicExceptions.cpp
    BasicExceptions.hpp
    BoostIostreams.hpp
    BoostFilesystem.hpp
    BoostAnyConversion.cpp
    BoostAnyConversion.hpp
    BoostAssertions.hpp
    BoostAssertions.cpp
    BoostArray.hpp
    BuildInfo.hpp
    BuildInfo.cpp
    CF.hpp
    Action.hpp
    Action.cpp
    ActionDirector.hpp
    ActionDirector.cpp
    GroupActions.hpp
    GroupActions.cpp
    Builder.hpp
    Builder.cpp
    CodeLocation.cpp
    CodeLocation.hpp
    CodeProfiler.cpp
    CodeProfiler.hpp
    CommonAPI.hpp
    Component.hpp
    Component.cpp
    FindComponents.hpp
    Environment.cpp
    Environment.hpp
    Factory.hpp
    Factory.cpp
    CFactories.hpp
    CFactories.cpp
    Root.hpp
    Root.cpp
    Group.hpp
    Group.cpp
    CJournal.cpp
    CJournal.hpp
    CLibraries.cpp
    CLibraries.hpp
    CLibrary.cpp
    CLibrary.hpp
    Link.hpp
    Link.cpp
    CMap.hpp
    Core.hpp
    Core.cpp
    CreateComponentDataType.hpp
    EigenAssertions.hpp
    EnumT.hpp
    EventHandler.hpp
    EventHandler.cpp
    ConnectionManager.hpp
    ConnectionManager.cpp
    Exception.cpp
    Exception.hpp
    Foreach.hpp
    IAction.hpp
    LibCommon.cpp
    LibCommon.hpp
    LibLoader.cpp
    LibLoader.hpp
    LocalDispatcher.cpp
    LocalDispatcher.hpp
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
    NetworkInfo.cpp
    NetworkInfo.hpp
    NoProfiling.cpp
    NoProfiling.hpp
    NotificationQueue.cpp
    NotificationQueue.hpp
    Option.cpp
    Option.hpp
    OptionArray.cpp
    OptionArray.hpp
    OptionT.hpp
    OptionT.cpp
    OptionList.cpp
    OptionList.hpp
    OptionURI.cpp
    OptionURI.hpp
    OptionComponent.hpp
    PropertyList.hpp
    PropertyList.cpp
    OSystem.cpp
    OSystem.hpp
    OSystemLayer.cpp
    OSystemLayer.hpp
    RegistLibrary.hpp
    StreamHelpers.hpp
    StringConversion.hpp
    StringConversion.cpp
    Signal.hpp
    Signal.cpp
    SignalDispatcher.cpp
    SignalDispatcher.hpp
    SignalHandler.hpp
    SignalHandler.cpp
    TaggedObject.hpp
    TaggedObject.cpp
    Tags.hpp
    Tags.cpp
    TimedComponent.hpp
    TimedComponent.cpp
    Timer.cpp
    Timer.hpp
    TypeInfo.cpp
    TypeInfo.hpp
    URI.hpp
    URI.cpp
    WorkerStatus.cpp
    WorkerStatus.hpp

    XML/CastingFunctions.cpp
    XML/CastingFunctions.hpp
    XML/FileOperations.cpp
    XML/FileOperations.hpp
    XML/Map.cpp
    XML/Map.hpp
    XML/MultiArray.cpp
    XML/MultiArray.hpp
    XML/Protocol.cpp
    XML/Protocol.hpp
    XML/SignalFrame.cpp
    XML/SignalFrame.hpp
    XML/SignalOptions.cpp
    XML/SignalOptions.hpp
    XML/XmlDoc.cpp
    XML/XmlDoc.hpp
    XML/XmlNode.cpp
    XML/XmlNode.hpp
)

###############################################################################
# MPI Files
if(CF3_HAVE_MPI)
list( APPEND coolfluid_common_files
      PE/Buffer.hpp
      PE/CPEManager.cpp
      PE/CPEManager.hpp
      PE/CWorkerGroup.cpp
      PE/CWorkerGroup.hpp
      PE/ListeningInfo.cpp
      PE/ListeningInfo.hpp
      PE/ListeningThread.cpp
      PE/ListeningThread.hpp
      PE/Comm.hpp
      PE/Comm.cpp
      PE/CommWrapper.cpp
      PE/CommWrapper.hpp
      PE/CommWrapperMArray.hpp
      PE/CommWrapperMArray.cpp
      PE/CommPattern.hpp
      PE/CommPattern.cpp
      PE/datatype.hpp
      PE/operations.hpp
      PE/debug.hpp
      PE/debug.cpp
      PE/scatter.hpp
      PE/gather.hpp
      PE/all_gather.hpp
      PE/all_to_all.hpp
      PE/all_reduce.hpp
      PE/broadcast.hpp
      PE/reduce.hpp
      PE/types.hpp
)
endif()

###############################################################################
# Operating System dependent files

### UNIXES ####
if(UNIX)

  if( CF3_HAVE_DLOPEN )
    list( APPEND coolfluid_common_files PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp )
    list( APPEND coolfluid_common_libs  dl )
    set( CF3_HAVE_LIBLOADER ON )
  endif()

  if( CF3_OS_MACOSX )
    list( APPEND coolfluid_common_files MacOSX/OSystemLayer.hpp   MacOSX/OSystemLayer.cpp )
  endif()

  if( CF3_OS_LINUX )
    list( APPEND coolfluid_common_files Linux/OSystemLayer.hpp   Linux/OSystemLayer.cpp )
  endif()

endif()

### WINDOWS ####
if(WIN32)

  list( APPEND coolfluid_common_files Win32/OSystemLayer.hpp   Win32/OSystemLayer.cpp )

  # should search explicitly for dbghelp.dll
  # should search for psapi.dll if not in windows 7 (already in kernel32.dll)

  # libraries for OSystemLayerWin32
  list( APPEND coolfluid_common_libs dbghelp psapi )

  if( CF3_HAVE_WINDOWSH )
    list( APPEND coolfluid_common_files Win32/LibLoader.hpp     Win32/LibLoader.cpp )
    set( CF3_HAVE_LIBLOADER ON )
  endif()

endif()

###############################################################################

if( NOT CF3_HAVE_LIBLOADER )
  message( FATAL_ERROR "No dynamic library loader was found!")
endif()

coolfluid_mark_not_orphan(
  PosixDlopenLibLoader.hpp  PosixDlopenLibLoader.cpp
  MacOSX/OSystemLayer.hpp   MacOSX/OSystemLayer.cpp
  Linux/OSystemLayer.hpp    Linux/OSystemLayer.cpp
  Win32/OSystemLayer.hpp    Win32/OSystemLayer.cpp
  Win32/LibLoader.hpp       Win32/LibLoader.cpp
)

###############################################################################

list( APPEND coolfluid_common_libs ${CF3_BOOST_LIBRARIES})

# explicit linking of all libraries for modern linkers (gcc4.5)
if( CF3_HAVE_RT )
  list( APPEND coolfluid_common_libs ${RT_LIBRARIES})
endif()

# faster allocation and memory porfiling
if( CF3_ENABLE_TCMALLOC )
  list(APPEND coolfluid_common_libs ${GOOGLEPERFTOOLS_TCMALLOC_LIBRARY} )
endif()

set( coolfluid_common_kernellib TRUE )

coolfluid_add_library( coolfluid_common )

add_subdirectory( MacOSX )   # support for Mac OSX
add_subdirectory( Linux  )   # support for Linux
add_subdirectory( Win32  )   # support for Win32

