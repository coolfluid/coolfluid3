#############################################################
# ParaView plugin for the GUI

#if( ParaView_FOUND )

  # List of classes to "moc"
  list( APPEND coolfluid_ui_qwt_tab_moc_classes
    BodePlot.hpp
    ColorSelector.hpp
    Graph.hpp
    GraphOption.hpp
    NPlotXY.hpp
   )

  # Create the moc files
  QT4_WRAP_CPP(coolfluid_ui_qwt_tab_moc_files ${coolfluid_ui_qwt_tab_moc_classes})

  # Add the project files
  list( APPEND coolfluid_ui_qwt_tab_files
    BodePlot.cpp
    BodePlot.hpp
    ColorSelector.cpp
    ColorSelector.hpp
    LibQwtTab.cpp
    LibQwtTab.hpp
    Graph.hpp
    Graph.cpp
    GraphOption.cpp
    GraphOption.hpp
    NPlotXY.cpp
    NPlotXY.hpp
    PixMaps.hpp
  )

  include( ${QT_USE_FILE} )
  list( APPEND coolfluid_ui_qwt_tab_cflibs coolfluid_ui_graphics )

  # Use Qt Libraries
  list( APPEND coolfluid_ui_qwt_tab_libs ${QT_LIBRARIES} coolfluid_qwt )

  coolfluid_add_library(coolfluid_ui_qwt_tab)

#else()

#   coolfluid_mark_not_orphan(
#    LibQwtTab.cpp
#    LibQwtTab.hpp
#   )

#endif()
