#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# Add the project files (including the moc files)

list( APPEND coolfluid-client_files main.cpp  )

if( APPLE )
  #set( coolfluid-client_make_bundle TRUE)

  set( MACOSX_BUNDLE_INFO_STRING "Client application for COOLFluiD" )

  # set the bundle icon file
  set( MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/MacOSX_Bundle/coolfluid.icns)
  set_source_files_properties(MACOSX_BUNDLE_ICON_FILE PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  # bundle name (the name that appears in the menu bar, next to the apple)
  set( MACOSX_BUNDLE_BUNDLE_NAME "COOLFluiD client")

endif()

add_subdirectory( Core )
add_subdirectory( UI )

# Use Qt Libraries
list( APPEND coolfluid-client_libs coolfluid_client_ui ${QT_LIBRARIES})

coolfluid_add_application( coolfluid-client )
