coolfluid_find_orphan_files()

list( APPEND coolfluid_common_files
    Action.hpp
    Action.cpp
    ActionDirector.hpp
    ActionDirector.cpp
    AllocatedComponent.hpp
    AllocatedComponent.cpp
    ArrayBase.hpp
    ArrayBufferT.hpp
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
    BoostArray.cpp
    Builder.hpp
    Builder.cpp
    BuildInfo.hpp
    BuildInfo.cpp
    CF.hpp
    CodeLocation.cpp
    CodeLocation.hpp
    CodeProfiler.cpp
    CodeProfiler.hpp
    CommonAPI.hpp
    Component.hpp
    Component.cpp
    ComponentIterator.hpp
    ConnectionManager.hpp
    ConnectionManager.cpp
    Core.hpp
    Core.cpp
    CreateComponentDataType.hpp
    DynTable.hpp
    DynTable.cpp
    EigenAssertions.hpp
    EnumT.hpp
    Environment.cpp
    Environment.hpp
    EventHandler.hpp
    EventHandler.cpp
    Exception.cpp
    Exception.hpp
    Factory.hpp
    Factory.cpp
    Factories.hpp
    Factories.cpp
    FindComponents.hpp
    Foreach.hpp
    Group.hpp
    Group.cpp
    Handle.hpp
    IAction.hpp
    Journal.cpp
    Journal.hpp
    LibCommon.cpp
    LibCommon.hpp
    LibLoader.cpp
    LibLoader.hpp
    Libraries.cpp
    Libraries.hpp
    Library.cpp
    Library.hpp
    Link.hpp
    Link.cpp
    List.hpp
    List.cpp
    ListBufferT.hpp
    ListBufferIterator.hpp
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
    Map.hpp
    NetworkInfo.cpp
    NetworkInfo.hpp
    NoProfiling.cpp
    NoProfiling.hpp
    NotificationQueue.cpp
    NotificationQueue.hpp
    Option.cpp
    Option.hpp
    OptionArray.hpp
    OptionArray.cpp
    OptionArrayDetail.hpp
    OptionFactory.hpp
    OptionFactory.cpp
    OptionT.hpp
    OptionT.cpp
    OptionList.cpp
    OptionList.hpp
    OptionListDetail.hpp
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
    Table_fwd.hpp
    Table.hpp
    Table.cpp
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
    UUCount.hpp
    UUCount.cpp
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
      PE/Manager.cpp
      PE/Manager.hpp
      PE/WorkerGroup.cpp
      PE/WorkerGroup.hpp
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
if( CF3_HAVE_REALTIME )
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

