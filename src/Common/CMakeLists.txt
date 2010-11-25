coolfluid_find_orphan_files()

list( APPEND coolfluid_common_files
    AssertionManager.cpp
    AssertionManager.hpp
    BasicExceptions.cpp
    BasicExceptions.hpp
    BoostIostreams.hpp
    BoostArray.hpp
    BuildInfo.hpp
    BuildInfo.cpp
    CF.hpp
    CBuilder.hpp
    CodeLocation.cpp
    CodeLocation.hpp
    CodeProfiler.cpp
    CodeProfiler.hpp
    CommonAPI.hpp
    Component.hpp
    Component.cpp
    ComponentIterator.hpp
    ComponentPredicates.hpp
    CEnv.cpp
    CEnv.hpp
    CFactory.hpp
    CFactory.cpp
    CFactories.hpp
    CFactories.cpp
    CPath.hpp
    CPath.cpp
    CRoot.hpp
    CRoot.cpp
    CGroup.hpp
    CGroup.cpp
    CLibraries.cpp
    CLibraries.hpp
    CLibrary.cpp
    CLibrary.hpp
    CLink.hpp
    CLink.cpp
    CMap.hpp
    ConfigArgs.cpp
    ConfigArgs.hpp
    Core.hpp
    Core.cpp
    CreateComponent.hpp
    DirPaths.cpp
    DirPaths.hpp
    EnumT.hpp
    EventHandler.hpp
    EventHandler.cpp
    Exception.cpp
    Exception.hpp
    ExportAPI.hpp
    Foreach.hpp
    GeneralStorage.hpp
    HourMinSec.cpp
    HourMinSec.hpp
    LibCommon.cpp
    LibCommon.hpp
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
    NamedObject.hpp
    NamedObject.cpp
    NonInstantiable.hpp
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
    OptionURI.cpp
    OptionURI.hpp
    Property.hpp
    Property.cpp
    PropertyList.hpp
    PropertyList.cpp
    ConfigObject.hpp
    ConfigObject.cpp
    OSystem.cpp
    OSystem.hpp
    OSystemLayer.cpp
    OSystemLayer.hpp
    RegistLib.hpp
    Stopwatch.hpp
    StreamHelpers.hpp
    String/Conversion.hpp
    String/Conversion.cpp
    SignalHandler.hpp
    SignalHandler.cpp
    TaggedObject.hpp
    TaggedObject.cpp
    TimePolicies.cpp
    TimePolicies.hpp
    TypeInfo.cpp
    TypeInfo.hpp
    URI.hpp
    URI.cpp
    WorkerStatus.cpp
    WorkerStatus.hpp
    XmlHelpers.hpp
    XmlHelpers.cpp
    XmlSignature.cpp
    XmlSignature.hpp
    XML.hpp
    XML.cpp
)

###############################################################################
# MPI Files
if(CF_HAVE_MPI)
list( APPEND coolfluid_common_files
      MPI/PE.hpp
      MPI/PE.cpp
      MPI/PEObjectWrapper.cpp
      MPI/PEObjectWrapper.hpp
      MPI/PECommPattern.hpp
      MPI/PECommPattern.cpp
      MPI/PECommPattern2.hpp
      MPI/PECommPattern2.cpp
      MPI/tools.hpp
      MPI/scatter.hpp
      MPI/all_to_all.hpp
)
endif()

###############################################################################
# Operating System dependent files

### UNIXES ####
if(UNIX)

  if( CF_HAVE_DLOPEN )
    list( APPEND coolfluid_common_files PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp )
    list( APPEND coolfluid_common_libs  dl )
    set( CF_HAVE_LIBLOADER ON )
  endif()

  if( CF_OS_MACOSX )
    list( APPEND coolfluid_common_files MacOSX/OSystemLayer.hpp   MacOSX/OSystemLayer.cpp )
  endif()

  if( CF_OS_LINUX )
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

  if( CF_HAVE_WINDOWSH )
    list( APPEND coolfluid_common_files Win32/LibLoader.hpp     Win32/LibLoader.cpp )
    set( CF_HAVE_LIBLOADER ON )
  endif()

endif()

###############################################################################

if( NOT CF_HAVE_LIBLOADER )
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

list( APPEND coolfluid_common_libs ${Boost_LIBRARIES})

# explicit linking of all libraries for modern linkers (gcc4.5)
if( CF_HAVE_RT )
  list( APPEND coolfluid_common_libs ${RT_LIBRARY})
endif()

# faster allocation and memory porfiling
if( CF_USE_TCMALLOC )
  list(APPEND coolfluid_common_libs ${GOOGLE_PERFTOOLS_TCMALLOC_LIB} )
endif()

set( coolfluid_common_kernellib TRUE )

coolfluid_add_library( coolfluid_common )

add_subdirectory( MacOSX )   # support for Mac OSX
add_subdirectory( Linux  )   # support for Linux
add_subdirectory( Win32  )   # support for Win32

add_subdirectory( String  )  # std::string conversions

add_subdirectory( uTests )    # unit tests for Common

