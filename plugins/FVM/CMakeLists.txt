##############################################################################
# FVM plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( FVM CXX )

coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # testing tree


