##############################################################################
# RiemannSolvers plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( RiemannSolvers CXX )

coolfluid_define_plugin( RiemannSolvers ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF3_PLUGIN_RIEMANNSOLVERS )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  add_subdirectory( RiemannSolvers )  # RiemannSolvers library

  add_subdirectory( test ) # testing tree

endif()
