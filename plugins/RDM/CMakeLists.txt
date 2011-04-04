##############################################################################
# RDM plugin
##############################################################################

# TODO
#
# now ...
#
# * euler system - jets testcase
# * make outputting convergence info an action
# * stop condition based on norm and/or iterations
# * use the iterate action - forward euler , rungekutta
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
# * physical model abstract class w self-registered implementations
# * configuration of linear adv speed
# * automatic bubble enrichment in rk case
# * neuman bc

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( RDM CXX )

coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set( RDM_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources CACHE INTERNAL "Dir for resources" )
set( RDM_TEST_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/test      CACHE INTERNAL "Dir for tests" )

add_subdirectory( src )  # library sources

add_subdirectory( test ) # unit and acceptance tests

add_subdirectory( resources ) # resources : test case meshes, etc
