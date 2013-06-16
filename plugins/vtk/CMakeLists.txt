##############################################################################
# VTK Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( VTK CXX )

coolfluid_define_plugin( VTK ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(VTK QUIET)

if( CF3_PLUGIN_VTK AND VTK_FOUND )
  include(${VTK_USE_FILE})
  
  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

  add_subdirectory( src )  # library sources

  add_subdirectory( test ) # testing tree

endif()