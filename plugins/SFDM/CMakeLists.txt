##############################################################################
# Spectral Finite Difference plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( SFDM CXX )

coolfluid_define_plugin( SFDM ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF_PLUGIN_SFDM )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  add_subdirectory( SFDM )  # library sources

  add_subdirectory( test ) # testing tree

endif()
