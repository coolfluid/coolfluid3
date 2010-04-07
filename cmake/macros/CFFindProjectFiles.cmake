##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
MACRO ( CF_LIST_PROJECT_FILES aFileList )

# first find all the files in the directory
foreach( CFEXT ${CF_FILE_EXTENSIONS} )

    file ( GLOB_RECURSE listFilesWithExt *.${CFEXT})
    
    list ( LENGTH  listFilesWithExt sizeFilesWithExt )
    if ( sizeFilesWithExt GREATER 0 )
      set ( ${aFileList} ${${aFileList}} ${listFilesWithExt} )
    endif ()
  
endforeach()

ENDMACRO ()
##############################################################################

##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
MACRO ( CF_MARK_NOT_ORPHAN )

  # remove files marked as not orphan
  foreach( AFILE ${ARGV} )
	set ( thisFileName ${CMAKE_CURRENT_SOURCE_DIR}/${AFILE} )
    list ( REMOVE_ITEM CF_ORPHAN_FILES ${thisFileName} )
  endforeach()

  # rewrite the orphan file list in cache
  set ( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} CACHE INTERNAL "" FORCE )

ENDMACRO ()
##############################################################################

##############################################################################
# finds project files and adds them to the passed variable
##############################################################################
FUNCTION ( CF_FIND_ORPHAN_FILES )

	CF_LIST_PROJECT_FILES ( cwdFiles )

	# append found files to orphan files (will be removed later as they are used)
	set ( CF_PROJECT_FILES ${CF_PROJECT_FILES} ${cwdFiles} CACHE INTERNAL "" FORCE )
	
	# append found files to orphan files (will be removed later as they are used)
	set ( CF_ORPHAN_FILES ${CF_ORPHAN_FILES} ${cwdFiles} CACHE INTERNAL "" FORCE )

ENDFUNCTION ()
##############################################################################


