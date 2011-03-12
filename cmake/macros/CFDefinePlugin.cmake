##############################################################################
# macro for defining a plugin
##############################################################################

macro( coolfluid_define_plugin PLUGIN_NAME PLUGIN_DIR )

      set( CF_PLUGIN_LIST ${CF_PLUGIN_LIST} ${PLUGIN_NAME} CACHE INTERNAL "" )
      coolfluid_log_file( "PLUGIN [${PLUGIN_NAME}]" )
      coolfluid_log_file( "+++ DIR [${PLUGIN_DIR}]" )

endmacro()

##############################################################################
