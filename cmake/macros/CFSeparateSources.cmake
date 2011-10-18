##############################################################################
# this macro separates the sources form the headers
##############################################################################
macro( coolfluid_separate_sources FILELIST TGTNAME )

  foreach( AFILE ${FILELIST} )

    set( thisFileName ${CMAKE_CURRENT_SOURCE_DIR}/${AFILE} )

    # check for existance of all declared files
    if( EXISTS ${thisFileName} )

      # check that file lenght is not too big
      coolfluid_check_file_length(${AFILE})

      # separate headers
      if(${AFILE} MATCHES "(\\.hh|\\.ci|\\.h|\\.hpp|\\.py|\\.cfscript)")
        list( APPEND ${TGTNAME}_headers ${AFILE})
        list( REMOVE_ITEM CF_ORPHAN_FILES ${thisFileName} )
      endif()

      # separate sources
      if(${AFILE} MATCHES "(\\.cxx|\\.cpp|\\.cc|\\.c|\\.f|\\.f77|\\.f90)")
        list( APPEND ${TGTNAME}_sources ${AFILE})
        list( REMOVE_ITEM CF_ORPHAN_FILES ${thisFileName} )
      endif()


    else()

      message( FATAL_ERROR "In directory ${CMAKE_CURRENT_SOURCE_DIR} file ${AFILE} was declared in CMakeLists.txt but not found" )

    endif()

    # rewrite the orphan file list in cache
    set( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} CACHE INTERNAL "" )

  endforeach()

endmacro()
##############################################################################

