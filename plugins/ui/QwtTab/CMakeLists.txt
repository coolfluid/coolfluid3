#############################################################
# ParaView plugin for the GUI

# classes to "moc"
list( APPEND coolfluid_ui_qwt_tab_moc_classes
    BodePlot.hpp
    ColorSelector.hpp
    Graph.hpp
    GraphOption.hpp
    NPlotXY.hpp
)

# project files
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

if( QT4_FOUND )
    include( ${QT_USE_FILE} )
endif()
  
coolfluid3_add_library( TARGET     coolfluid_ui_qwt_tab
                        SOURCES
                            ${coolfluid_ui_qwt_tab_files}
                        MOC
                            ${coolfluid_ui_qwt_tab_moc_classes}
                        LIBS
					  		coolfluid_ui_graphics ${QT_LIBRARIES} coolfluid_qwt
                        CONDITION
					  		QT4_FOUND )
