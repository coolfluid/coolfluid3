##############################################################################
# adds a path to search for when searching for include files
##############################################################################
macro( ADD_TRIAL_INCLUDE_PATH ICPATH )
 if( EXISTS ${ICPATH})
  list( APPEND TRIAL_INCLUDE_PATHS ${ICPATH} )
 endif( EXISTS ${ICPATH})
endmacro( ADD_TRIAL_INCLUDE_PATH )
##############################################################################

##############################################################################
# adds a path to search for when searching for library files
##############################################################################
macro( ADD_TRIAL_LIBRARY_PATH ICPATH )
 if( EXISTS ${ICPATH})
   list( APPEND TRIAL_LIBRARY_PATHS ${ICPATH} )
 endif( EXISTS ${ICPATH})
endmacro( ADD_TRIAL_LIBRARY_PATH )
##############################################################################

##############################################################################
# sets a path to search for when searching for INCLUDE files
##############################################################################
macro( SET_TRIAL_INCLUDE_PATH IPATHS )
 set( TRIAL_INCLUDE_PATHS "" )
 foreach( path ${IPATHS} )
   if( EXISTS ${path})
     list( APPEND TRIAL_INCLUDE_PATHS ${path} )
   endif( EXISTS ${path})
 endforeach( path ${IPATHS} )
endmacro( SET_TRIAL_INCLUDE_PATH )
##############################################################################

##############################################################################
# sets a path to search for when searching for library files
##############################################################################
macro( SET_TRIAL_LIBRARY_PATH LPATHS )
 set( TRIAL_LIBRARY_PATHS "" )
 foreach( path ${LPATHS} )
   if( EXISTS ${path})
     list( APPEND TRIAL_LIBRARY_PATHS ${path} )
   endif( EXISTS ${path})
 endforeach( path ${LPATHS} )
endmacro( SET_TRIAL_LIBRARY_PATH )
##############################################################################
