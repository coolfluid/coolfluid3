if( QT4_FOUND AND CF_ENABLE_GUI )

  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})

  # coolfluid_log("  QT_INCLUDES  : ${QT_INCLUDES}")
  # coolfluid_log("  QT_LIBRARIES : ${QT_LIBRARIES}")


  # sub project dirs
  # include_directories( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES} )

  add_subdirectory( Client )
  add_subdirectory( Network )
  add_subdirectory( Server )

endif()

