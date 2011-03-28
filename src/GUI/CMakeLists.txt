if( QT4_FOUND AND CF_ENABLE_GUI )

  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})



  add_subdirectory( Core  )
  add_subdirectory( Graphics  )

  add_subdirectory( UICommon )
  add_subdirectory( Server  )

  #####################################################################
  # Graphical application

  list( APPEND coolfluid-gui_files gui-main.cpp )
  list( APPEND coolfluid-gui_cflibs coolfluid_ui_graphics)

  list( APPEND coolfluid-gui_libs ${QT_LIBRARIES} )

  coolfluid_add_application( coolfluid-gui )

endif()

