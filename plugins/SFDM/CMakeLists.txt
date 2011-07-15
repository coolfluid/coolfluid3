##############################################################################
# Spectral Finite Difference plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( SFDM CXX )

coolfluid_define_plugin( SFDM ${CMAKE_CURRENT_SOURCE_DIR} )

coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # testing tree

