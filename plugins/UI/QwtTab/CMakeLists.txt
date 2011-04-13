#############################################################
# ParaView plugin for the GUI

#if( ParaView_FOUND )

  # List of classes to "moc"
  list( APPEND coolfluid_ui_qwt_tab_moc_classes
      Widget3D.hpp
   )

  # Create the moc files
 # QT4_WRAP_CPP(coolfluid_ui_qwt_tab_moc_files ${coolfluid_ui_qwt_tab_moc_classes})

  # Add the project files
  list( APPEND coolfluid_ui_qwt_tab_files
    LibQwtTab.cpp
    LibQwtTab.hpp
  )

 # include_directories(${PARAVIEW_INCLUDE_DIRS})

  list( APPEND coolfluid_ui_qwt_tab_cflibs coolfluid_ui_graphics )


  include( ${QT_USE_FILE} )

  # Use Qt Libraries
  list( APPEND coolfluid_ui_qwt_tab_libs ${QT_LIBRARIES} coolfluid_qwt )

#QT4_ADD_RESOURCES(coolfluid_ui_qwt_tab_RCC Resources/paraview.qrc)

  coolfluid_add_library(coolfluid_ui_qwt_tab)

#else()

#   coolfluid_mark_not_orphan(
#    LibParaView.cpp
#    LibParaView.hpp
#    Widget3D.cpp
#    Widget3D.hpp
#   )

#endif()
