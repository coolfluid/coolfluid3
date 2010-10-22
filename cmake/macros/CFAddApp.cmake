##############################################################################
# macro for adding a application in the project
##############################################################################
macro( coolfluid_add_application APPNAME )

  # option to build it or not
  option( CF_BUILD_${APPNAME} "Build the ${APPNAME} application" ON )

  # add to list of local apps
  list( APPEND CF_LOCAL_APPNAMES ${APPNAME} )

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

    add_executable( ${APPNAME} ${${APPNAME}_sources} ${${APPNAME}_headers} ${${APPNAME}_moc_files} ${${APPNAME}_RCC})

    # add installation paths
    INSTALL( TARGETS ${APPNAME}
      RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
      LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
      ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
      )

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
    if( CF_ENABLE_PROFILING AND CF_PROFILER_IS_GOOGLE AND CF_BUILD_GooglePerfTools )
      list( APPEND ${APPNAME}_cflibs GooglePerfTools )
    endif()

    # profiling selected for specific target
    if( ${APPNAME}_profile AND CF_BUILD_GooglePerfTools )
      list( APPEND ${APPNAME}_cflibs coolfluid_google_perftools )
    endif()

    # internal dependencies
    if( DEFINED ${APPNAME}_cflibs )
        TARGET_LINK_LIBRARIES ( ${APPNAME} ${${APPNAME}_cflibs} )
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
  coolfluid_log_file("${APPNAME}_all_mods_pres   : [${${APPNAME}_all_mods_pres}]")
  coolfluid_log_file("${APPNAME}_requires_mods   : [${${APPNAME}_requires_mods}]")
  coolfluid_log_file("${APPNAME}_P_SOURCES       : [${${APPNAME}_P_SOURCES}]")
  coolfluid_log_file("${APPNAME}_LINK_FLAGS      : [${${APPNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${APPNAME}_TYPE            : [${${APPNAME}_TYPE}]")


endmacro( coolfluid_add_application )
##############################################################################
