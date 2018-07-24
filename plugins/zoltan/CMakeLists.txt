##############################################################################
# Zoltan Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( zoltan CXX )

coolfluid_define_plugin( zoltan ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(Zoltan QUIET)
if(CF3_PLUGIN_ZOLTAN AND CF3_HAVE_ZOLTAN)
  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

  add_subdirectory( src )  # library sources

  add_subdirectory( test ) # testing tree

endif()