#############################################################
# ParaView plugin for the GUI

# classes to "moc"
list( APPEND coolfluid_ui_paraview_moc_classes  C3DView.hpp )

# Add the project files
list( APPEND coolfluid_ui_paraview_files
  C3DView.cpp
  C3DView.hpp
  C3DViewBuilder.cpp
  C3DViewBuilder.hpp
  LibParaView.cpp
  LibParaView.hpp
)

coolfluid3_add_library( TARGET    coolfluid_ui_paraview
                        SOURCES
						  	${coolfluid_ui_paraview_files} 
                        MOC
                            ${coolfluid_ui_paraview_moc_classes}
                        LIBS
						  	${CF3_KERNEL_LIBS} ${QT_LIBRARIES} ${ParaView_LIBRARIES}
                        INCLUDES
						  	${PARAVIEW_INCLUDE_DIRS}
                        CONDITION
                            ParaView_FOUND AND QT4_FOUND )
