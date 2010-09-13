if( QT4_FOUND AND CF_ENABLE_GUI )

  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  #  we need XML and Network modules
  #  set( QT_USE_QTXML TRUE     )
  #  set( QT_USE_QTNETWORK TRUE )

  # Add useful macros and variables
  include( ${QT_USE_FILE} )

  # sub project dirs
  INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES} )

  add_subdirectory( Client )
  add_subdirectory( Network )
  add_subdirectory( Server )

endif()

