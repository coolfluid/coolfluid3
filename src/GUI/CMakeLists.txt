IF (QT4_FOUND)

  CF_FIND_ORPHAN_FILES()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    SET ( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  #  we need XML and Network modules
  #  SET( QT_USE_QTXML TRUE     )
  #  SET( QT_USE_QTNETWORK TRUE )

  # Add useful macros and variables
  INCLUDE( ${QT_USE_FILE} )

  # sub project dirs
  INCLUDE_DIRECTORIES ( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES} )

  ADD_SUBDIRECTORY ( Client )
  ADD_SUBDIRECTORY ( Network )
# ADD_SUBDIRECTORY ( Server )

ENDIF()
