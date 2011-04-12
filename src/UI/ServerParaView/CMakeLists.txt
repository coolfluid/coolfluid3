#############################################################
# ParaView plugin for the GUI

if( ParaView_FOUND )

  # List of classes to "moc"
  list( APPEND coolfluid_ui_serverparaview_moc_classes
    C3DView.hpp
    C3DViewBuilder.hpp
   )

  # Create the moc files
  QT4_WRAP_CPP(coolfluid_ui_serverparaview_moc_files ${coolfluid_ui_paraview_moc_classes})

  # Add the project files
  list( APPEND coolfluid_ui_serverparaview_files
	C3DView.cpp
	C3DView.hpp
	C3DViewBuilder.cpp
	C3DViewBuilder.hpp
  LibServerParaView.cpp
  LibServerParaView.hpp
  )

  include_directories(${PARAVIEW_INCLUDE_DIRS})

  list( APPEND coolfluid_ui_serverparaview_cflibs coolfluid_ui_graphics )



  # Use Qt Libraries
  list( APPEND coolfluid_ui_serverparaview_libs ${QT_LIBRARIES} ${ParaView_LIBRARIES})

  coolfluid_add_library(coolfluid_ui_serverparaview)

else()

   coolfluid_mark_not_orphan(
	C3DView.cpp
	C3DView.hpp
	C3DViewBuilder.cpp
	C3DViewBuilder.hpp
   )

endif()
