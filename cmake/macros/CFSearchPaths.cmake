##############################################################################
# adds a path to search for when searching for include files
##############################################################################
macro( coolfluid_add_trial_include_path ICPATH )
 if( EXISTS ${ICPATH})
  list( APPEND TRIAL_INCLUDE_PATHS ${ICPATH} )
 endif( EXISTS ${ICPATH})
endmacro( coolfluid_add_trial_include_path )
##############################################################################

##############################################################################
# adds a path to search for when searching for library files
##############################################################################
macro( coolfluid_add_trial_library_path ICPATH )
 if( EXISTS ${ICPATH})
   list( APPEND TRIAL_LIBRARY_PATHS ${ICPATH} )
 endif( EXISTS ${ICPATH})
endmacro( coolfluid_add_trial_library_path )
##############################################################################

##############################################################################
# sets a path to search for when searching for INCLUDE files
##############################################################################
macro( coolfluid_set_trial_include_path IPATHS )
 set( TRIAL_INCLUDE_PATHS "" )
 foreach( path ${IPATHS} )
   if( EXISTS ${path})
     list( APPEND TRIAL_INCLUDE_PATHS ${path} )
   endif( EXISTS ${path})
 endforeach( path ${IPATHS} )
endmacro( coolfluid_set_trial_include_path )
##############################################################################

##############################################################################
# sets a path to search for when searching for library files
##############################################################################
macro( coolfluid_set_trial_library_path LPATHS )
 set( TRIAL_LIBRARY_PATHS "" )
 foreach( path ${LPATHS} )
   if( EXISTS ${path})
     list( APPEND TRIAL_LIBRARY_PATHS ${path} )
   endif( EXISTS ${path})
 endforeach( path ${LPATHS} )
endmacro( coolfluid_set_trial_library_path )
##############################################################################


##############################################################################
# logs the final result of a dependency search
##############################################################################
macro( coolfluid_log_deps_result LIBNAME )

if( NOT CF_SKIP_${LIBNAME} )

  set( CF_HAVE_${LIBNAME} 1 )

  # all vars must be defined and found

  foreach( VAR ${ARGN} )
    if(DEFINED ${VAR})
      mark_as_advanced( ${ARGN} ) # advanced marking

      if( NOT ${VAR} ) # found ?
        set( CF_HAVE_${LIBNAME} 0 )
      endif()

    else() # not defined
      set( CF_HAVE_${LIBNAME} 0 )
    endif()
  endforeach() # var

  # set CF_HAVE in cache

  if(CF_HAVE_${LIBNAME})
    set(CF_HAVE_${LIBNAME} 1 CACHE BOOL "Found dependency ${LIBNAME}")
    if(DEFINED ${LIBNAME}_LIBRARIES})
      list( APPEND CF_DEPS_LIBRARIES ${{LIBNAME}_LIBRARIES} )
    endif()
  else()
    set(CF_HAVE_${LIBNAME} 0 CACHE BOOL "Did not find dependency ${LIBNAME}")
  endif()

  # logging

  coolfluid_log( "CF_HAVE_${LIBNAME}: [${CF_HAVE_${LIBNAME}}]" )
  if(CF_HAVE_${LIBNAME})
    foreach( VAR ${ARGN} ) # log to file
      coolfluid_log_file( "  ${VAR}:  [${${VAR}}]" )
    endforeach()
  endif()

else()

    coolfluid_log( "CF_HAVE_${LIBNAME}: - searched skipped" )
    set(CF_HAVE_${LIBNAME} 0 CACHE BOOL "Skipped dependency ${LIBNAME}")

endif() # skip

endmacro( coolfluid_log_deps_result )
##############################################################################
