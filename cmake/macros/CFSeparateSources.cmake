##############################################################################
# this macro separates the sources form the headers
##############################################################################
MACRO( CF_SEPARATE_SOURCES FILELIST TGTNAME )

  FOREACH( AFILE ${FILELIST} )

    set ( thisFileName ${CMAKE_CURRENT_SOURCE_DIR}/${AFILE} )
	  
    # check for existance of all declared files
    IF ( EXISTS ${thisFileName} )

      # check that file lenght is not too big
      CF_CHECK_FILE_LENGTH(${AFILE})

      # separate headers
      IF(${AFILE} MATCHES "(\\.hh|\\.ci|\\.h|\\.hpp)")
        LIST ( APPEND ${TGTNAME}_headers ${AFILE})
	LIST( REMOVE_ITEM CF_ORPHAN_FILES ${thisFileName} )
      ENDIF()

      # separate sources
      IF(${AFILE} MATCHES "(\\.cxx|\\.cpp|\\.cc|\\.c|\\.f|\\.f77|\\.f90)")
        LIST ( APPEND ${TGTNAME}_sources ${AFILE})
	LIST( REMOVE_ITEM CF_ORPHAN_FILES ${thisFileName} )
      ENDIF()


    ELSE ()

      MESSAGE ( FATAL_ERROR "In directory ${CMAKE_CURRENT_SOURCE_DIR} file ${AFILE} was declared in CMakeLists.txt but not found" )
    
    ENDIF ()

    # rewrite the orphan file list in cache
    SET ( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} CACHE INTERNAL "" FORCE )
  
  ENDFOREACH()

ENDMACRO( CF_SEPARATE_SOURCES )
##############################################################################

