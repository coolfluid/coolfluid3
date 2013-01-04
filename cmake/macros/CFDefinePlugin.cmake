##############################################################################
# macro for defining a plugin
##############################################################################

macro( coolfluid_define_plugin PLUGIN_NAME PLUGIN_DIR )

  coolfluid_log_file( "PLUGIN [${PLUGIN_NAME}]" )
  coolfluid_log_file( "+++ DIR [${PLUGIN_DIR}]" )

  string( TOUPPER ${PLUGIN_NAME} PLUGIN_NAME_CAPS )

  # set( ${PLUGIN_NAME}_DIR ${PLUGIN_DIR} PARENT_SCOPE )
  set( ${PLUGIN_NAME}_DIR ${PLUGIN_DIR} CACHE INTERNAL "" )

  option( CF3_PLUGIN_${PLUGIN_NAME_CAPS}  "Enable the plugin ${PLUGIN_NAME}" ON  )

  if( CF3_PLUGIN_${PLUGIN_NAME_CAPS} )
    set( CF3_PLUGIN_LIST ${CF3_PLUGIN_LIST} ${PLUGIN_NAME} CACHE INTERNAL "" )
  endif()

endmacro()

##############################################################################
