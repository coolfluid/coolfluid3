# if file with orphan files exists remove it
SET ( ORPHAN_FILE "${coolfluid_BINARY_DIR}/OrphanFiles.txt" )
IF ( EXISTS ${ORPHAN_FILE} ) 
	FILE ( REMOVE ${ORPHAN_FILE} )
ENDIF()


# if orphan files where found, put the list on the file
LIST ( LENGTH CF_ORPHAN_FILES CF_LENGTH_ORPHAN_FILES)
IF ( CF_LENGTH_ORPHAN_FILES )

  LOG ( " !!! ERROR !!! ")
  LOG  ( "Orphan file list:" )
  FOREACH( AFILE ${CF_ORPHAN_FILES} )
    FILE ( APPEND ${ORPHAN_FILE} "${AFILE}\n" )
    LOG  ( "${AFILE}" )
  ENDFOREACH()

  LOG ( " !!! ERROR !!! ")
  LOG ( " !!! ERROR !!! Orphan files where found during the configuration.")
  LOG ( " !!! ERROR !!! Check full list in file ${ORPHAN_FILE}")
  LOG ( " !!! ERROR !!! ")

  MESSAGE ( FATAL_ERROR "\n Aborted build system configuration. \n Add orphan files to the build system or remove them." )


ENDIF()

