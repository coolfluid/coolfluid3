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
   LogStringDisplayer.cxx
   LogStringDisplayer.hh
   LogStringForwarder.hh
   CodeLocation.cxx
   CodeLocation.hh
   Common.hh
   ExportAPI.hh
   NonCopyable.hh
   NonInstanciable.hh
   PE.cxx
   PE.hh
   PEInterface.cxx
   PEInterface.hh
   StringOps.hh
   StringOps.cxx
   WorkerStatus.cxx
   WorkerStatus.hh)

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

SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

ADD_SUBDIRECTORY ( MacOSX )   # support for Mac OSX
ADD_SUBDIRECTORY ( Linux  )   # support for Linux
ADD_SUBDIRECTORY ( Win32  )   # support for Win32

ADD_SUBDIRECTORY(tests)

