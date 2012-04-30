##############################################################################
# Spectral Difference plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( spectral_difference CXX )

coolfluid_define_plugin( SDM ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF3_PLUGIN_SDM )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  add_subdirectory( sdm )  # library sources

  add_subdirectory( test ) # testing tree

  add_subdirectory( executables ) # executables

endif()
