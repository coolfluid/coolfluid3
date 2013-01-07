#############################################################
# ParaView plugin for the GUI

# List of classes to "moc"
list( APPEND coolfluid_ui_paraview_moc_classes
	C3DView.hpp
)

# Create the moc files
if( ParaView_FOUND AND QT4_FOUND )
  	QT4_WRAP_CPP(coolfluid_ui_paraview_moc_files ${coolfluid_ui_paraview_moc_classes})
endif()

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
						  	${coolfluid_ui_paraview_moc_files} 
					      LIBS      
						  	${CF3_KERNEL_LIBS} ${QT_LIBRARIES} ${ParaView_LIBRARIES}
					      INCLUDES  
						  	${PARAVIEW_INCLUDE_DIRS}
					      CONDITION 
					  		ParaView_FOUND AND QT4_FOUND ) 
