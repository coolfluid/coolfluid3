##############################################################################
# RDM plugin
##############################################################################

# TODO
#
# now ...
#
# * make update solution an action
# * make compute norm an action
# * make outputting convergence info an action
# * use the generic iterate with ordered actions
# * port rkrd time stepping from CF2
# * linearized euler equations
#
# soon ...
#
# * P3 triangle
# * P3 quad
# * reformulate the linear operator common functions within the scheme
# * reformulate the interface to the physics
# * supg scheme
# * bubble scheme
#
# later ...
#
# * physical model user side configuration
# * configuration of linear adv speed
# * automatic bubble enrichment in rk case
# * euler system
# * neuman bc

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( RDM CXX )

coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # testing tree
