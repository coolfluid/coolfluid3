if( QT4_FOUND AND CF3_ENABLE_GUI )

  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})


  add_subdirectory( network )

  add_subdirectory( core  )
  add_subdirectory( graphics  )
#  add_subdirectory( ServerParaView )

  add_subdirectory( uicommon )
  add_subdirectory( server  )

  #####################################################################
  # Graphical application

  list( APPEND coolfluid-gui_files gui-main.cpp )
  list( APPEND coolfluid-gui_cflibs coolfluid_ui_graphics)

  # link against Qt libraries and QwtTab plugin
  list( APPEND coolfluid-gui_libs ${QT_LIBRARIES} coolfluid_ui_qwt_tab )

  if( ParaView_FOUND )
    add_definitions( -DCF3_HAVE_PARAVIEW )
    include_directories( ${PARAVIEW_INCLUDE_DIRS} )
    list( APPEND coolfluid-gui_libs coolfluid_ui_paraview_tab  )
  endif()

  coolfluid_add_application( coolfluid-gui )

  #####################################################################
  # Server application

  list( APPEND coolfluid-server_files server-main.cpp )
  list( APPEND coolfluid-server_cflibs coolfluid_ui_server)

  # link against Qt libraries and QwtTab plugin
  list( APPEND coolfluid-server_libs ${QT_LIBRARIES} coolfluid_ui_server )

#  if( ParaView_FOUND )
#    add_definitions( -DCF3_HAVE_PARAVIEW )
#    include_directories( ${PARAVIEW_INCLUDE_DIRS} )
#    list( APPEND coolfluid-server_libs coolfluid_ui_paraview )
#  endif()

  coolfluid_add_application( coolfluid-server )


endif()

