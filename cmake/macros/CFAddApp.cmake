##############################################################################
# macro for adding a application in the project
##############################################################################

macro( coolfluid_add_application APPNAME )

  # option to build it or not (option is basic)
  option( CF3_BUILD_${APPNAME} "Build the ${APPNAME} application" ON )

  # by default, applications are not part of the sandbox
  if( NOT DEFINED ${APPNAME}_sandbox_app )
    set( ${APPNAME}_sandbox_app OFF )
  endif()

  #
  #
  #
  #
  #
  #
  #

  # check if all required modules are present
  set( ${APPNAME}_has_all_plugins TRUE )
  foreach( req_plugin ${${APPNAME}_requires_plugins} )
    list( FIND CF3_PLUGIN_LIST ${req_plugin} pos )
    if( ${pos} EQUAL -1 )
      set( ${APPNAME}_has_all_plugins FALSE )
      if( CF3_BUILD_${APPNAME} )
          coolfluid_log_verbose( "     \# APP [${APPNAME}] requires plugin [${req_plugin}] which is not present")
      endif()
    endif()
  endforeach()

  set( ${APPNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  # separate the source files and remove them from the orphan list

  coolfluid_separate_sources("${${APPNAME}_files}" ${APPNAME})

  source_group( Headers FILES ${${APPNAME}_headers} )
  source_group( Sources FILES ${${APPNAME}_sources} )

  # set condition if not set outside, default is TRUE
  if( DEFINED  ${APPNAME}_condition )
    coolfluid_log_verbose("${APPNAME} has condition [${${APPNAME}_condition}]")
  else()
    set( ${APPNAME}_condition TRUE )
  endif()

  if( CF3_BUILD_${APPNAME} AND ${APPNAME}_has_all_plugins AND ${APPNAME}_condition )
    set( ${APPNAME}_builds YES CACHE INTERNAL "" )
  else()
    set( ${APPNAME}_builds NO  CACHE INTERNAL "" )
  endif()

  # compile if selected and all required modules are present
  if(${APPNAME}_builds )

    coolfluid_log_file( " +++ APP   [${APPNAME}]" )

    if( DEFINED ${APPNAME}_includedirs )
      include_directories(${${APPNAME}_includedirs})
    endif()

    # check if the application should be Mac OS bundle ("*.app" directory)
    if( ${APPNAME}_make_bundle )

      if( NOT APPLE )
        message( WARNING "Bundle applications can only be built under Mac OS.")
      endif()

      # directories to look for dependencies
      SET(DIRS ${BOOST_ROOT})

      # tell cmake to creat a bundle
      SET( ${APPNAME}_platform MACOSX_BUNDLE ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/coolfluid.icns)

      # system identifier (should be universally unique)
      SET( MACOSX_BUNDLE_GUI_IDENTIFIER "be.ac.vki.${APPNAME}")

      # tell cpack the destination of the bundle when generating the installer
      # package
      SET( ${APPNAME}_specific_destination BUNDLE DESTINATION ${CF3_INSTALL_BIN_DIR} )

      # set the long and short version strings
      SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${CF3_VERSION} (Kernel ${CF3_KERNEL_VERSION})")
      SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${CF3_VERSION}" )
      #  SET( MACOSX_BUNDLE_BUNDLE_VERSION )

      #  SET( MACOSX_BUNDLE_COPYRIGHT )

      set( ${APPNAME}_fixup_bundle "include(BundleUtilities)
        fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/coolfluid-client.app\" \"${CF3_BOOST_LIBRARIES}\"
                     \"\")" )

      coolfluid_log_verbose("${APPNAME} application will be built as a Mac OS bundle.")

    endif()

    add_executable( ${APPNAME} ${${APPNAME}_platform} ${${APPNAME}_sources} ${${APPNAME}_headers} ${${APPNAME}_moc_files} ${${APPNAME}_RCC})

    # if mpi was found add it to the libraries
    if(CF3_HAVE_MPI AND NOT CF3_HAVE_MPI_COMPILER)
      target_link_libraries( ${APPNAME} ${MPI_LIBRARIES} )
      if( DEFINED MPI_CXX_LIBRARIES )
          target_link_libraries( ${APPNAME} ${MPI_CXX_LIBRARIES} )
      endif()
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${APPNAME}_libs )
      #list(REMOVE_DUPLICATES ${APPNAME}_libs)
      target_link_libraries( ${APPNAME} ${${APPNAME}_libs} )
    endif()

    # internal dependencies
    if( DEFINED ${APPNAME}_cflibs )
        list(REMOVE_DUPLICATES ${APPNAME}_cflibs)
        target_link_libraries( ${APPNAME} ${${APPNAME}_cflibs} )
    endif()

    if( ${APPNAME}_make_bundle )

      if( NOT ${APPNAME}_sandbox_app )
        install( TARGETS ${APPNAME}
          RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR} COMPONENT applications
          LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT applications
          ARCHIVE DESTINATION ${CF3_INSTALL_ARCHIVE_DIR} COMPONENT applications
          BUNDLE DESTINATION ${CF3_INSTALL_BIN_DIR} COMPONENT applications
        )

      endif()

    else()
      # add installation paths, if it not a sandbox application
      if( NOT ${APPNAME}_sandbox_app )
        install( TARGETS ${APPNAME}
          RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR} COMPONENT applications
          LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT applications
          ARCHIVE DESTINATION ${CF3_INSTALL_ARCHIVE_DIR} COMPONENT applications
        )
      endif()

    endif()

  endif()

  get_target_property( ${APPNAME}_P_SOURCES        ${APPNAME} SOURCES )
  get_target_property( ${APPNAME}_LINK_FLAGS       ${APPNAME} LINK_FLAGS )
  get_target_property( ${APPNAME}_TYPE             ${APPNAME} TYPE )

  # log some info about the app
  coolfluid_log_file("${APPNAME} user option     : [${CF3_BUILD_${APPNAME}}]")
  coolfluid_log_file("${APPNAME}_builds          : [${${APPNAME}_builds}]")
  coolfluid_log_file("${APPNAME}_dir             : [${${APPNAME}_dir}]")
  coolfluid_log_file("${APPNAME}_includedirs     : [${${APPNAME}_includedirs}]")
  coolfluid_log_file("${APPNAME}_libs            : [${${APPNAME}_libs}]")
  coolfluid_log_file("${APPNAME}_cflibs          : [${${APPNAME}_cflibs}]")
  coolfluid_log_file("${APPNAME}_sandbox_app     : [${${APPNAME}_sandbox_app}]")
  coolfluid_log_file("${APPNAME}_has_all_plugins : [${${APPNAME}_has_all_plugins}]")
  coolfluid_log_file("${APPNAME}_requires_plugins: [${${APPNAME}_requires_plugins}]")
  coolfluid_log_file("${APPNAME}_P_SOURCES       : [${${APPNAME}_P_SOURCES}]")
  coolfluid_log_file("${APPNAME}_LINK_FLAGS      : [${${APPNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${APPNAME}_TYPE            : [${${APPNAME}_TYPE}]")

  #coolfluid_install_targets( ${APPNAME} )

endmacro( coolfluid_add_application )
##############################################################################
