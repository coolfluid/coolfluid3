##############################################################################
# this macro logs simultaneously to screen and to file
##############################################################################
MACRO ( coolfluid_log line )
   MESSAGE(STATUS ${line})
   FILE(APPEND ${PROJECT_LOG_FILE} "${line}\n")
ENDMACRO ()

##############################################################################
# this macro logs just to file
##############################################################################
MACRO ( coolfluid_log_file line )
   FILE(APPEND ${PROJECT_LOG_FILE} "${line}\n")
ENDMACRO ( )
##############################################################################

##############################################################################
# this macro logs to screen if we are verbose
##############################################################################
MACRO ( coolfluid_log_verbose line )
    IF ( CF_CMAKE_VERBOSE )
      coolfluid_log( ${line} )
    ELSE ()
      coolfluid_log_file( ${line} )
    ENDIF()
ENDMACRO ( )
##############################################################################


