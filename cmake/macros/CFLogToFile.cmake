##############################################################################
# this macro logs simultaneously to screen and to file
##############################################################################
macro( coolfluid_log line )
   message(STATUS ${line})
   file(APPEND ${PROJECT_LOG_FILE} "${line}\n")
endmacro()

##############################################################################
# this macro logs just to file
##############################################################################
macro( coolfluid_log_file line )
   file(APPEND ${PROJECT_LOG_FILE} "${line}\n")
endmacro()
##############################################################################

##############################################################################
# this macro logs to screen if we are verbose
##############################################################################
macro( coolfluid_log_verbose line )
    if( CF3_CMAKE_VERBOSE )
      coolfluid_log( ${line} )
    else()
      coolfluid_log_file( ${line} )
    endif()
endmacro()
##############################################################################


