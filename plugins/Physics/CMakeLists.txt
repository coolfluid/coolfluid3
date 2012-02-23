##############################################################################
# Physics plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( Physics CXX )

coolfluid_define_plugin( Physics ${CMAKE_CURRENT_SOURCE_DIR} )

if( CF3_PLUGIN_PHYSICS )

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  # library NavierStokes
  add_subdirectory( NavierStokes )

  # library Scalar
  add_subdirectory( Scalar )

  # library LinEuler
  add_subdirectory( LinEuler )

  # testing tree
  add_subdirectory( test )

endif()
