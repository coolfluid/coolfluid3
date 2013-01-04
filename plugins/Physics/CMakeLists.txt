##############################################################################
# Physics plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( Physics CXX )

coolfluid_define_plugin( Physics ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF3_PLUGIN_PHYSICS )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  add_subdirectory( cf3 )

  # testing tree
  add_subdirectory( test )

endif()
