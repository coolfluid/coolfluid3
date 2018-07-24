##############################################################################
# VTK Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( VTK CXX )

find_package(ParaView 4.2 QUIET COMPONENTS vtkFiltersCore vtkPVCatalyst)

if(ParaView_FOUND)
  coolfluid_define_plugin( VTK ${CMAKE_CURRENT_SOURCE_DIR} )
endif()


if( CF3_PLUGIN_VTK AND ParaView_FOUND)
  include("${PARAVIEW_USE_FILE}")
  add_definitions("-DUSE_CATALYST")
  if(NOT PARAVIEW_USE_MPI)
    message(SEND_ERROR "ParaView must be built with MPI enabled")
  endif()
  
  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

  add_subdirectory( src )  # library sources

  add_subdirectory( test ) # testing tree

endif()
