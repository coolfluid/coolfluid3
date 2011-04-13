if( QT4_FOUND AND CF_ENABLE_GUI )

  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})

  coolfluid_define_plugin( QwtTab "${CMAKE_CURRENT_SOURCE_DIR}/QwtTab" )
  add_subdirectory( QwtTab )

  coolfluid_define_plugin( ParaViewTab "${CMAKE_CURRENT_SOURCE_DIR}/ParaViewTab" )
  add_subdirectory( ParaViewTab )

  coolfluid_define_plugin( ParaView "${CMAKE_CURRENT_SOURCE_DIR}/ParaView" )
  add_subdirectory( ParaView )

endif()
