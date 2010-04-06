##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
MACRO ( CF_LIST_PROJECT_FILES aFileList )

# first find all the files in the directory
FOREACH( CFEXT ${CF_FILE_EXTENSIONS} )

    FILE ( GLOB_RECURSE listFilesWithExt *.${CFEXT})
    
    LIST ( LENGTH  listFilesWithExt sizeFilesWithExt )
    IF ( sizeFilesWithExt GREATER 0 )
      SET ( ${aFileList} ${${aFileList}} ${listFilesWithExt} )
    ENDIF ()
  
ENDFOREACH()

ENDMACRO ( CF_LIST_PROJECT_FILES )
##############################################################################

##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
MACRO ( CF_MARK_NOT_ORPHAN )

  # remove files marked as not orphan
  foreach( AFILE ${ARGV} )
    list ( REMOVE_ITEM CF_ORPHAN_FILES ${AFILE} )
  endforeach()

  # rewrite the orphan file list in cache
  set ( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} CACHE INTERNAL "" FORCE )

ENDMACRO ( CF_LIST_PROJECT_FILES )
##############################################################################

##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
FUNCTION ( CF_FIND_ORPHAN_FILES )

	CF_LIST_PROJECT_FILES ( cwdFiles )

	# append found files to orphan files (will be removed later as they are used)
	SET ( CF_PROJECT_FILES ${CF_PROJECT_FILES} ${cwdFiles} CACHE INTERNAL "" FORCE )
	
	# append found files to orphan files (will be removed later as they are used)
	SET ( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} ${cwdFiles} CACHE INTERNAL "" FORCE )

ENDFUNCTION ( CF_FIND_ORPHAN_FILES )
##############################################################################


