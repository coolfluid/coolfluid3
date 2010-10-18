#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# Add the project files (including the moc files)
list( APPEND coolfluid-client_files main.cpp  )

add_subdirectory( Core )
add_subdirectory( UI )

# Use Qt Libraries
list( APPEND coolfluid-client_libs coolfluid_client_ui ${QT_LIBRARIES})

coolfluid_add_application( coolfluid-client )

add_subdirectory( uTests )
