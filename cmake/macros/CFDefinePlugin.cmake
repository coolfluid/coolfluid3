##############################################################################
# macro for defining a plugin
##############################################################################

macro( coolfluid_define_plugin PLUGIN_NAME PLUGIN_DIR )

      set( CF_PLUGIN_LIST ${CF_PLUGIN_LIST} ${PLUGIN_NAME} CACHE INTERNAL "" )
      coolfluid_log( "PLUGIN [${PLUGIN_NAME}] DIR [${PLUGIN_DIR}]" )

endmacro()

##############################################################################
