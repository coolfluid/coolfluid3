coolfluid_find_orphan_files()

list( APPEND Common_files
    AssertionManager.cpp
    AssertionManager.hpp
    BasicExceptions.cpp
    BasicExceptions.hpp
    BoostIostreams.hpp
    BoostArray.hpp
    CF.hpp
    CodeLocation.cpp
    CodeLocation.hpp
    CodeProfiler.cpp
    CodeProfiler.hpp
    CommonAPI.hpp
    Component.hpp
    Component.cpp
    ComponentIterator.hpp
    ComponentPredicates.hpp
    CPath.hpp
    CPath.cpp
    CRoot.hpp
    CRoot.cpp
    CGroup.hpp
    CGroup.cpp
    CLibrary.cpp
    CLibrary.hpp
    CLink.hpp
    CLink.cpp
    ConcreteProvider.hpp
    ConfigArgs.cpp
    ConfigArgs.hpp
    Core.hpp
    Core.cpp
    CoreVars.hpp
    CoreVars.cpp
    DirPaths.cpp
    DirPaths.hpp
    EnumT.hpp
    EventHandler.hpp
    EventHandler.cpp
    Exception.cpp
    Exception.hpp
    ExportAPI.hpp
    Factory.hpp
    FactoryRegistry.hpp
    FactoryRegistry.cpp
    GeneralStorage.hpp
    HourMinSec.cpp
    HourMinSec.hpp
    LibCommon.cpp
    LibCommon.hpp
    LibLoader.cpp
    LibLoader.hpp
    LibraryRegistry.hpp
    LibraryRegisterBase.hpp
    LibraryRegistry.cpp
    LibraryRegister.hpp
    LibraryRegisterBase.cpp
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
    ObjectProvider.hpp
    Option.cpp
    Option.hpp
    OptionArray.cpp
    OptionArray.hpp
    OptionT.hpp
    Property.hpp
    Property.cpp
    PropertyList.hpp
    PropertyList.cpp
    OptionComponent.hpp
    ConfigObject.hpp
    ConfigObject.cpp
    OSystem.cpp
    OSystem.hpp
    OwnedObject.hpp
    OSystemLayer.cpp
    OSystemLayer.hpp
    Provider.hpp
    ProviderBase.cpp
    ProviderBase.hpp
    SafePtr.hpp
    SharedPtr.hpp
    SetupObject.hpp
    SetupObject.cpp
    SelfRegistry.cpp
    SelfRegistry.hpp
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
list( APPEND Common_files
      MPI/PEInterface.hpp
      MPI/PEInterface.cpp
      MPI/PECommPattern.hpp
      MPI/PECommPattern.cpp
      MPI/scatterv.hpp
)
endif()

###############################################################################
# Operating System dependent files

### UNIXES ####
if(UNIX)

  if( CF_HAVE_DLOPEN )
    list( APPEND Common_files PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp )
    list( APPEND Common_libs  dl )
    set( CF_HAVE_LIBLOADER ON )
  endif()

  if( CF_OS_MACOSX )
    list( APPEND Common_files MacOSX/OSystemLayer.hpp   MacOSX/OSystemLayer.cpp )
  endif()

  if( CF_OS_LINUX )
    list( APPEND Common_files Linux/OSystemLayer.hpp   Linux/OSystemLayer.cpp )
  endif()

endif()

### WINDOWS ####
if(WIN32)

  list( APPEND Common_files Win32/OSystemLayer.hpp   Win32/OSystemLayer.cpp )

  # should search explicitly for dbghelp.dll
  # should search for psapi.dll if not in windows 7 (already in kernel32.dll)

  # libraries for OSystemLayerWin32
  list( APPEND Common_libs dbghelp psapi )

  if( CF_HAVE_WINDOWSH )
    list( APPEND Common_files Win32/LibLoader.hpp     Win32/LibLoader.cpp )
    set( CF_HAVE_LIBLOADER ON )
  endif()

endif()

###############################################################################

if( NOT CF_HAVE_LIBLOADER )
  message( FATAL_ERROR "No dynamic library loader was found!")
endif()

coolfluid_mark_not_orphan (
PosixDlopenLibLoader.hpp PosixDlopenLibLoader.cpp
MacOSX/OSystemLayer.hpp   MacOSX/OSystemLayer.cpp
Linux/OSystemLayer.hpp    Linux/OSystemLayer.cpp
Win32/OSystemLayer.hpp    Win32/OSystemLayer.cpp
Win32/LibLoader.hpp      Win32/LibLoader.cpp
)


###############################################################################

list( APPEND Common_libs ${Boost_LIBRARIES})

# faster allocation and memory porfiling
if( CF_USE_TCMALLOC )
  list(APPEND Common_libs ${GOOGLE_PERFTOOLS_TCMALLOC_LIB} )
endif()

set( Common_kernellib TRUE )

coolfluid_add_library( Common )

add_subdirectory( MacOSX )   # support for Mac OSX
add_subdirectory( Linux  )   # support for Linux
add_subdirectory( Win32  )   # support for Win32

add_subdirectory( String  )  # std::string conversions

add_subdirectory( uTests )    # unit tests for Common

