coolfluid_find_orphan_files()

add_subdirectory( network )
add_subdirectory( uicommon )
add_subdirectory( server )

#####################################################################
# Server application


list( APPEND coolfluid-server_files server-main.cpp )
list( APPEND coolfluid-server_cflibs coolfluid_ui_server)

list( APPEND coolfluid-server_libs coolfluid_ui_server )

configure_file( ssh_simple_tunnel.tcl   ${CMAKE_CURRENT_BINARY_DIR}/ssh_simple_tunnel.tcl   @ONLY)

configure_file( ssh_reverse_tunnel.tcl   ${CMAKE_CURRENT_BINARY_DIR}/ssh_reverse_tunnel.tcl   @ONLY)

coolfluid_add_application( coolfluid-server )

#####################################################################
# Qt-dependent stuff

if( CF3_ENABLE_GUI AND QT4_FOUND ) # add cmake Qt macros and variables

    include(${QT_USE_FILE})

    # deactivate some c++ warnings in Qt
    if( CMAKE_COMPILER_IS_GNUCC )
      set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
    endif()

endif()

    add_subdirectory( core  )
    add_subdirectory( graphics  )

    #####################################################################
    # Graphical application

    list( APPEND coolfluid-gui_files  gui-main.cpp )
    list( APPEND coolfluid-gui_cflibs coolfluid_ui_graphics)

    # link against Qt libraries and QwtTab plugin
    list( APPEND coolfluid-gui_libs ${QT_LIBRARIES} coolfluid_ui_qwt_tab )

    if( ParaView_FOUND )
      add_definitions( -DCF3_HAVE_PARAVIEW )
      include_directories( ${PARAVIEW_INCLUDE_DIRS} )
      list( APPEND coolfluid-gui_libs coolfluid_ui_paraview_tab  )
    endif()

    if( CF3_ENABLE_GUI AND QT4_FOUND )
        set( coolfluid-gui_condition TRUE )
    else()
        set( coolfluid-gui_condition FALSE )
    endif()

    coolfluid_add_application( coolfluid-gui )

#else()

    #file( GLOB_RECURSE UI_FIND_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")

    #coolfluid_mark_not_orphan( ${UI_FIND_FILES} gui-main.cpp )

#endif( CF3_ENABLE_GUI AND QT4_FOUND )

