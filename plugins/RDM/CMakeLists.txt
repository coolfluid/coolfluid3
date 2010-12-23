##############################################################################
# RDM plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( RDM CXX )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # testing tree


