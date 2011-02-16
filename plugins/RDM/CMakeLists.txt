##############################################################################
# RDM plugin
##############################################################################

# TODO
#
# soon ...
# * simplify configuration
# * P3 triangle
# * P3 quad
# * n scheme
# * reformulate the linear operator common functions within the scheme
# * reformulate the interface to the physics
# * supg scheme
# * bubble scheme
# * rk time stepping
#
# later ...
# * physical model user side configuration
# * configuration of linear adv speed
# * automatic bubble enrichment in rk case
# * euler system
# * neuman bc
# * linearized euler equations

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( RDM CXX )

coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # testing tree
