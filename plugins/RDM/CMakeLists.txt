##############################################################################
# RDM plugin
##############################################################################

# TODO
#
# soon ...
#
# * residual n wave_speed cleanup is rdm::action with configurable fields
# * solution initialization is an action
# * obtain cfl from option and/or separate component
# * P3 triangle
# * P3 quad
# * reformulate the linear operator common functions within the scheme
# * reformulate the interface to the physics
# * supg scheme
# * bubble scheme
# * rk time stepping
#
# later ...
#
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
