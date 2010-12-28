##############################################################################
# macro for adding a application in the project
##############################################################################
macro( coolfluid_add_application APPNAME )

  # option to build it or not
  option( CF_BUILD_${APPNAME} "Build the ${APPNAME} application" ON )

  # add to list of local apps
  list( APPEND CF_LOCAL_APPNAMES ${APPNAME} )

  # by default, applications are not part of the sandbox
  if( NOT DEFINED ${APPNAME}_sandbox_app )
    set( ${APPNAME}_sandbox_app OFF )
  endif()

  # by default, applications are built as Mac OS bundle
  if( NOT DEFINED ${APPNAME}_sandbox_app )
    set( ${APPNAME}_sandbox_app OFF )
  endif()

#   coolfluid_debug_var(CF_MODULES_LIST)

  # check if all required modules are present
  set( ${APPNAME}_all_mods_pres ON )
  foreach( reqmod ${${APPNAME}_requires_mods} )
    list( FIND CF_MODULES_LIST ${reqmod} pos )
    if( ${pos} EQUAL -1 )
      set( ${APPNAME}_all_mods_pres OFF )
      if( CF_BUILD_${APPNAME} )
          coolfluid_log_verbose( "     \# app [${APPNAME}] requires module [${reqmod}] which is not present")
      endif()
    endif()
  endforeach()

  set( ${APPNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  if( CF_BUILD_${APPNAME} AND ${APPNAME}_all_mods_pres)
    set( ${APPNAME}_will_compile ON )
  else()
    set( ${APPNAME}_will_compile OFF )
  endif()

  coolfluid_log_verbose("app_${APPNAME} = ${${APPNAME}_will_compile}")

  # compile if selected and all required modules are present
  if(${APPNAME}_will_compile)

    if( DEFINED ${APPNAME}_includedirs )
      INCLUDE_DIRECTORIES(${${APPNAME}_includedirs})
    endif()

    coolfluid_separate_sources("${${APPNAME}_files}" ${APPNAME})

    source_group( Headers FILES ${${APPNAME}_headers} )
    source_group( Sources FILES ${${APPNAME}_sources} )

    coolfluid_log( " +++ APP   [${APPNAME}]" )

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
      SET( ${APPNAME}_specific_destination BUNDLE DESTINATION ${CF_INSTALL_BIN_DIR} )

      # set the long and short version strings
      SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${CF_VERSION} (Kernel ${CF_KERNEL_VERSION})")
      SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${CF_VERSION}" )
      #  SET( MACOSX_BUNDLE_BUNDLE_VERSION )

      #  SET( MACOSX_BUNDLE_COPYRIGHT )

      set( ${APPNAME}_fixup_bundle "include(BundleUtilities)
        fixup_bundle(\"${CMAKE_INSTALL_PREFIX}/coolfluid-client.app\" \"${CF_BOOST_LIBRARIES}\"
                     \"\")" )

      coolfluid_log_verbose("${APPNAME} application will be built as a Mac OS bundle.")

    endif()

    add_executable( ${APPNAME} ${${APPNAME}_platform} ${${APPNAME}_sources} ${${APPNAME}_headers} ${${APPNAME}_moc_files} ${${APPNAME}_RCC})

    # if mpi was found add it to the libraries
    if(CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)
#           message( STATUS "${APPNAME} links to ${MPI_LIBRARIES}" )
          TARGET_LINK_LIBRARIES ( ${APPNAME} ${MPI_LIBRARIES} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${APPNAME}_libs )
      TARGET_LINK_LIBRARIES ( ${APPNAME} ${${APPNAME}_libs} )
    endif()

    # profiling gloabally selected
    if( CF_ENABLE_PROFILING AND CF_PROFILER_IS_GOOGLE AND CF_BUILD_coolfluid_google_perftools )
      list( APPEND ${APPNAME}_cflibs coolfluid_google_perftools )
    endif()

    # profiling selected for specific target
    if( ${APPNAME}_profile AND CF_BUILD_coolfluid_google_perftools )
      list( APPEND ${APPNAME}_cflibs coolfluid_google_perftools )
    endif()

    # internal dependencies
    if( DEFINED ${APPNAME}_cflibs )
        TARGET_LINK_LIBRARIES ( ${APPNAME} ${${APPNAME}_cflibs} )
    endif()

    if( ${APPNAME}_make_bundle )

      if( NOT ${APPNAME}_sandbox_app )
        INSTALL( TARGETS ${APPNAME}
          RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR} COMPONENT applications
          LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR} COMPONENT applications
          ARCHIVE DESTINATION ${CF_INSTALL_ARCHIVE_DIR} COMPONENT applications
          BUNDLE DESTINATION ${CF_INSTALL_BIN_DIR} COMPONENT applications
        )

      endif()

    else()
      # add installation paths, if it not a sandbox application
      if( NOT ${APPNAME}_sandbox_app )
        INSTALL( TARGETS ${APPNAME}
          RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR} COMPONENT applications
          LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR} COMPONENT applications
          ARCHIVE DESTINATION ${CF_INSTALL_ARCHIVE_DIR} COMPONENT applications
        )
      endif()

    endif()

  endif()

  get_target_property ( ${APPNAME}_P_SOURCES        ${APPNAME} SOURCES )
  get_target_property ( ${APPNAME}_LINK_FLAGS       ${APPNAME} LINK_FLAGS )
  get_target_property ( ${APPNAME}_TYPE             ${APPNAME} TYPE )

  # log some info about the app
  coolfluid_log_file("${APPNAME} : [${CF_BUILD_${APPNAME}}]")
  coolfluid_log_file("${APPNAME} : [${${APPNAME}_will_compile}]")
  coolfluid_log_file("${APPNAME}_dir             : [${${APPNAME}_dir}]")
  coolfluid_log_file("${APPNAME}_includedirs     : [${${APPNAME}_includedirs}]")
  coolfluid_log_file("${APPNAME}_libs            : [${${APPNAME}_libs}]")
  coolfluid_log_file("${APPNAME}_cflibs          : [${${APPNAME}_cflibs}]")
  coolfluid_log_file("${APPNAME}_sandbox_app     : [${${APPNAME}_sandbox_app}]")
  coolfluid_log_file("${APPNAME}_all_mods_pres   : [${${APPNAME}_all_mods_pres}]")
  coolfluid_log_file("${APPNAME}_requires_mods   : [${${APPNAME}_requires_mods}]")
  coolfluid_log_file("${APPNAME}_P_SOURCES       : [${${APPNAME}_P_SOURCES}]")
  coolfluid_log_file("${APPNAME}_LINK_FLAGS      : [${${APPNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${APPNAME}_TYPE            : [${${APPNAME}_TYPE}]")

  #coolfluid_install_targets( ${APPNAME} )
  
endmacro( coolfluid_add_application )
##############################################################################
