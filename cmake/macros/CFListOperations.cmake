##############################################################################
# this macro adds to a cached list if element not yet present
##############################################################################
macro( coolfluid_remove_cached_list THELIST THEVAR )
  if( DEFINED ${THELIST} )
    list( REMOVE_ITEM ${THELIST} ${THEVAR} )
  endif( DEFINED ${THELIST} )
endmacro()
##############################################################################

##############################################################################
# this macro adds to a cached list if element not yet present
##############################################################################
macro( coolfluid_append_cached_list THELIST THEVAR )
  coolfluid_remove_cached_list(${THELIST} ${THEVAR})
  set( ${THELIST} ${${THELIST}} ${THEVAR} CACHE INTERNAL "" FORCE )
endmacro()
##############################################################################
