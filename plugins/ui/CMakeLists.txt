##############################################################################
# UI plugins
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( UI CXX )

if( QT4_FOUND AND CF3_ENABLE_GUI )

  coolfluid_find_orphan_files()

  coolfluid_define_plugin( UI ${CMAKE_CURRENT_SOURCE_DIR} )

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})

  add_subdirectory( QwtTab )

  add_subdirectory( ParaViewTab )

  add_subdirectory( ParaView )

endif()
