# Find Qt
# FIND_PACKAGE() defines QT_USE_FILE and QT_LIBRARIES used below
FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED)

IF (QT4_FOUND)

  SET ( CMAKE_C_FLAGS   ""   )
  SET ( CMAKE_CXX_FLAGS "" )

  # I need XML and Network modules
#  SET( QT_USE_QTXML TRUE     )
#  SET( QT_USE_QTNETWORK TRUE )

  # Add useful macros and variables
  INCLUDE( ${QT_USE_FILE} )
  
  # sub project dirs
  INCLUDE_DIRECTORIES ( ${CMAKE_SOURCE_DIR} ${QT_INCLUDES} )

  ADD_SUBDIRECTORY ( Client )
  ADD_SUBDIRECTORY ( Network )
  #ADD_SUBDIRECTORY ( Server )
  
ELSE()
  MESSAGE(FATAL_ERROR "Qt was not found")
ENDIF()
