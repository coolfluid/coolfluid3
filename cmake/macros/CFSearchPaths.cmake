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
# documents the feature search, for searches that already have an enabled var
##############################################################################
function( coolfluid_set_feature FEATURE_NOCAPS FEAT_ENABLE DESC )

  string( TOUPPER ${FEATURE_NOCAPS} FEATURE )

  if( COMMAND add_feature_info )
    add_feature_info( ${FEATURE_NOCAPS} FEAT_ENABLE ${DESC} )
  else()
    if( DEFINED ${FEATURE_NOCAPS}_FOUND )
      coolfluid_log( "${FEATURE_NOCAPS} [${${FEATURE_NOCAPS}_FOUND}]" )
    else()
      if( DEFINED ${FEATURE}_FOUND )
        coolfluid_log( "${FEATURE_NOCAPS} [${${FEATURE}_FOUND}]" )
      else()
        if( DEFINED CF3_HAVE_${FEATURE} )
          coolfluid_log( "${FEATURE_NOCAPS} [${CF3_HAVE_${FEATURE}}]" )
        endif()
      endif()
    endif()
  endif()

endfunction( coolfluid_set_feature )
##############################################################################

##############################################################################
# documents the package search, for searches that return Package_FOUND
##############################################################################
function( coolfluid_set_package )
# CMAKE_PARSE_ARGUMENTS(<prefix> <options> <one_value_keywords> <multi_value_keywords> args...)
  set( options ) # none
  set( oneValueArgs PACKAGE DESCRIPTION URL COMMENT )
  set( multiValueArgs ) # none

  cmake_parse_arguments(_P "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${ARGN})

  if(_P_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to coolfluid_set_package(): \"${_P_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT _P_PACKAGE)
    message(FATAL_ERROR "The call to coolfluid_set_package() doesn't set the required PACKAGE argument.")
  endif()

  string( TOUPPER ${_P_PACKAGE} PACKAGE )

#  coolfluid_debug_var(  _P_PACKAGE )
#  coolfluid_debug_var(  _P_DESCRIPTION )
#  coolfluid_debug_var(  _P_URL )

  if( COMMAND set_package_info )

    set_package_info( ${_P_PACKAGE} "${_P_DESCRIPTION}" "${_P_URL}" "${_P_COMMENT}" )

  else()

    if( DEFINED ${_P_PACKAGE}_FOUND )
      coolfluid_log( "${_P_PACKAGE} [${${_P_PACKAGE}_FOUND}]" )
    else()
      if( DEFINED ${PACKAGE}_FOUND )
        coolfluid_log( "${_P_PACKAGE} [${${PACKAGE}_FOUND}]" )
      else()
        if( DEFINED CF3_HAVE_${PACKAGE} )
          coolfluid_log( "${_P_PACKAGE} [${CF3_HAVE_${PACKAGE}}]" )
        endif()
      endif()
    endif()

  endif()

endfunction( coolfluid_set_package )
##############################################################################


##############################################################################
# documents the feature search
##############################################################################
function( coolfluid_add_package )
# CMAKE_PARSE_ARGUMENTS(<prefix> <options> <one_value_keywords> <multi_value_keywords> args...)
  set( options ) # none
  set( oneValueArgs PACKAGE DESCRIPTION URL )
  set( multiValueArgs VARS ) # none

  cmake_parse_arguments(_PAR "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${ARGN})

  if(_PAR_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to coolfluid_add_package(): \"${_PAR_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT _PAR_PACKAGE)
    message(FATAL_ERROR "The call to coolfluid_add_package() doesn't set the required PACKAGE argument.")
  endif()

  string( TOUPPER ${_PAR_PACKAGE} PACKAGE_CAPS )

  mark_as_advanced( ${_PAR_VARS} ) # advanced marking

  if( NOT CF3_SKIP_${PACKAGE_CAPS} )

  set( CF3_HAVE_${PACKAGE_CAPS} 1 )

  # all vars must be defined and found

  foreach( LISTVAR ${_PAR_VARS} )

    if(DEFINED ${LISTVAR}) # ignore variables that are not even defined (not searched for)

      foreach( VAR ${${LISTVAR}} )
        if( NOT VAR ) # found ?
          set( CF3_HAVE_${PACKAGE_CAPS} 0 )
        endif()
      endforeach() # var

    endif()

  endforeach() # listvar

  # set CF3_HAVE in cache

  if(CF3_HAVE_${PACKAGE_CAPS})
    set(CF3_HAVE_${PACKAGE_CAPS} 1 CACHE BOOL "Found dependency ${PACKAGE_CAPS}")
    if(DEFINED ${PACKAGE_CAPS}_LIBRARIES)
      list( APPEND CF3_DEPS_LIBRARIES ${${PACKAGE_CAPS}_LIBRARIES} )
    endif()
    if(DEFINED ${PACKAGE_CAPS}_EXTRA_LIBRARIES)
      list( APPEND CF3_DEPS_LIBRARIES ${${PACKAGE_CAPS}_EXTRA_LIBRARIES} )
    endif()
  else()
    set(CF3_HAVE_${PACKAGE_CAPS} 0 CACHE BOOL "Did not find dependency ${PACKAGE_CAPS}")
  endif()

  # logging

  coolfluid_log_file( "CF3_HAVE_${PACKAGE_CAPS}: [${CF3_HAVE_${PACKAGE_CAPS}}]" )
  if(CF3_HAVE_${PACKAGE_CAPS})
    foreach( LISTVAR ${ARGN} ) # log to file
      coolfluid_log_file( "  ${LISTVAR}:  [${${LISTVAR}}]" )
    endforeach()
  endif()

else()

    coolfluid_log_file( "CF3_HAVE_${PACKAGE_CAPS}: - searched skipped" )
    set(CF3_HAVE_${PACKAGE_CAPS} 0 CACHE BOOL "Skipped dependency ${PACKAGE_CAPS}")

endif() # skip

set( ${_PAR_PACKAGE}_FOUND ${CF3_HAVE_${PACKAGE_CAPS}} CACHE BOOL "${_PAR_PACKAGE} package" )
coolfluid_set_package( PACKAGE ${_PAR_PACKAGE} DESCRIPTION "${_PAR_DESCRIPTION}" URL "${_PAR_URL}" )

endfunction( coolfluid_add_package )
##############################################################################

