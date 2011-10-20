# if file with orphan files exists remove it
set( ORPHAN_FILE "${coolfluid_BINARY_DIR}/OrphanFiles.txt" )
if( EXISTS ${ORPHAN_FILE} ) 
	file( REMOVE ${ORPHAN_FILE} )
endif()


# if orphan files where found, put the list on the file
list( LENGTH CF3_ORPHAN_FILES CF3_LENGTH_ORPHAN_FILES)
if( CF3_LENGTH_ORPHAN_FILES )

  coolfluid_log( " !!! ERROR !!! ")
  coolfluid_log  ( "Orphan file list:" )
  foreach( AFILE ${CF3_ORPHAN_FILES} )
    file( APPEND ${ORPHAN_FILE} "${AFILE}\n" )
    coolfluid_log  ( "${AFILE}" )
  endforeach()

  coolfluid_log( " !!! ERROR !!! ")
  coolfluid_log( " !!! ERROR !!! Orphan files where found during the configuration.")
  coolfluid_log( " !!! ERROR !!! Check full list in file ${ORPHAN_FILE}")
  coolfluid_log( " !!! ERROR !!! ")

  message( FATAL_ERROR "\n Aborted build system configuration. \n Add orphan files to the build system or remove them." )


endif()

