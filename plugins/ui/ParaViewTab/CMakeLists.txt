#############################################################
# ParaView plugin for the GUI

list( APPEND coolfluid_ui_paraview_tab_moc_classes Widget3D.hpp N3DView.hpp )

list( APPEND coolfluid_ui_paraview_tab_files
    LibParaViewTab.cpp
    LibParaViewTab.hpp
    Widget3D.cpp
    Widget3D.hpp
    N3DView.cpp
    N3DView.hpp
)

if(ParaView_FOUND AND QT4_FOUND)
	QT4_WRAP_CPP(coolfluid_ui_paraview_tab_moc_files ${coolfluid_ui_paraview_tab_moc_classes})
	QT4_ADD_RESOURCES(coolfluid_ui_paraview_tab_RCC Resources/paraview.qrc)
endif()

  coolfluid3_add_library( 	TARGET   coolfluid_ui_paraview_tab
                          	SOURCES  
						  		${coolfluid_ui_paraview_tab_files} 
						  	MOC
								${coolfluid_ui_paraview_tab_moc_files} ${Resources/paraview.qrc} 
							RCC
							    ${coolfluid_ui_paraview_tab_RCC}
							INCLUDES 
								${PARAVIEW_INCLUDE_DIRS}
					      	LIBS
								coolfluid_ui_graphics ${QT_LIBRARIES} ${ParaView_LIBRARIES} 
							CONDITION
								ParaView_FOUND AND QT4_FOUND )
