##############################################################################
# prints the variable name and value
##############################################################################
macro( coolfluid_debug_var THE_VARIABLE )
  coolfluid_log( " +++++ DEBUG +++++ ${THE_VARIABLE} : [${${THE_VARIABLE}}]")
endmacro()

##############################################################################
# sets a variable if not yet defined
##############################################################################
macro( coolfluid_set_if_not_defined variable value )
  if( NOT DEFINED ${variable} )
    set( ${variable} ${value} )
  endif()
endmacro()
