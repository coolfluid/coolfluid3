##############################################################################
# UFEM plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( UFEM CXX )

coolfluid_define_plugin( UFEM ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF3_PLUGIN_UFEM AND CF3_ENABLE_PROTO )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

  add_subdirectory( src )  # library sources

  add_subdirectory( test ) # testing tree

endif()
