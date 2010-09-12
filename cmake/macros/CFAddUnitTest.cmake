##############################################################################
# macro for adding a testing application in the project
##############################################################################

macro( coolfluid_add_unittest UTESTNAME )

  # option to build it or not
  option( CF_BUILD_${UTESTNAME} "Build the ${UTESTNAME} testing application" ON )
  # this option is advanced (does not appear in the cmake gui)
  mark_as_advanced(CF_BUILD_${UTESTNAME})

  # add to list of local apps
  list( APPEND CF_LOCAL_UTESTNAMES ${UTESTNAME} )

#   coolfluid_debug_var(CF_MODULES_LIST)

  # check if all required modules are present
  set( ${UTESTNAME}_all_mods_pres ON )
  foreach( reqmod ${${UTESTNAME}_requires_mods} )
    list( FIND CF_MODULES_LIST ${reqmod} pos )
    if( ${pos} EQUAL -1 )
      set( ${UTESTNAME}_all_mods_pres OFF )
      if( CF_BUILD_${UTESTNAME} )
          coolfluid_log_verbose( "     \# utest [${UTESTNAME}] requires module [${reqmod}] which is not present")
      endif()
    endif()
  endforeach()

  # check that all CF dependencies will be compiling
  # else skip this uTest
  set( ${UTESTNAME}_all_cfdeps_ok ON )
  foreach( reqlib ${${UTESTNAME}_cflibs} )
    if( NOT ${${reqlib}_will_compile} )
       set( ${UTESTNAME}_all_cfdeps_ok OFF )
       if( CF_BUILD_${UTESTNAME} )
           coolfluid_log_verbose( "     \# utest [${UTESTNAME}] requires cflib [${reqlib}] which will not compile")
       endif()
    endif()
  endforeach()
  mark_as_advanced( ${UTESTNAME}_all_cfdeps_ok )

  set( ${UTESTNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  if( CF_BUILD_${UTESTNAME} AND ${UTESTNAME}_all_cfdeps_ok AND ${UTESTNAME}_all_mods_pres)
    set( ${UTESTNAME}_will_compile ON )
  else()
    set( ${UTESTNAME}_will_compile OFF )
  endif()

  coolfluid_log_verbose("test_${UTESTNAME} = ${${UTESTNAME}_will_compile}")

  # separate the source files
  # and remove them from the orphan list

  coolfluid_separate_sources("${${UTESTNAME}_files}" ${UTESTNAME})

  source_group( Headers FILES ${${UTESTNAME}_headers} )
  source_group( Sources FILES ${${UTESTNAME}_sources} )

  # compile if selected and all required modules are present
  if(${UTESTNAME}_will_compile)

    if( DEFINED ${UTESTNAME}_includedirs )
      INCLUDE_DIRECTORIES(${${UTESTNAME}_includedirs})
    endif()

    coolfluid_log( " +++ TEST [${UTESTNAME}]" )

    if( DEFINED ${UTESTNAME}_moc_files )
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers}  ${${UTESTNAME}_moc_files})
    else()
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers})
    endif()

    if(CF_INSTALL_TESTS)
      # add installation paths
      INSTALL( TARGETS ${UTESTNAME}
        RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
        )
    endif(CF_INSTALL_TESTS)

    # if mpi was found add it to the libraries
    if(CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)
#           message( STATUS "${UTESTNAME} links to ${MPI_LIBRARIES}" )
          TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${MPI_LIBRARIES} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${UTESTNAME}_libs )
      TARGET_LINK_LIBRARIES( ${UTESTNAME} ${${UTESTNAME}_libs} )
    endif(DEFINED ${UTESTNAME}_libs)

    # profiling gloabally selected
    if( CF_ENABLE_PROFILING AND CF_PROFILER_IS_GOOGLE AND CF_BUILD_GooglePerfTools )
      list( APPEND ${UTESTNAME}_cflibs GooglePerfTools )
    endif()

    # profiling selected for specific target
    if( ${UTESTNAME}_profile AND CF_BUILD_GooglePerfTools )
      list( APPEND ${UTESTNAME}_cflibs GooglePerfTools )
    endif()

    # internal dependencies
    if( DEFINED ${UTESTNAME}_cflibs )
        TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${${UTESTNAME}_cflibs} )
    endif()

  # add to the test database
  add_test( ${UTESTNAME} ${UTESTNAME} ${${UTESTNAME}_args})

  endif()

  get_target_property( ${UTESTNAME}_P_SOURCES   ${UTESTNAME} SOURCES )
  get_target_property( ${UTESTNAME}_LINK_FLAGS       ${UTESTNAME} LINK_FLAGS )
  get_target_property( ${UTESTNAME}_TYPE             ${UTESTNAME} TYPE )

  # log some info about the app
  coolfluid_log_file("${UTESTNAME} : [${CF_BUILD_${UTESTNAME}}]")
  coolfluid_log_file("${UTESTNAME} : [${${UTESTNAME}_will_compile}]")
  coolfluid_log_file("${UTESTNAME}_dir             : [${${UTESTNAME}_dir}]")
  coolfluid_log_file("${UTESTNAME}_includedirs     : [${${UTESTNAME}_includedirs}]")
  coolfluid_log_file("${UTESTNAME}_libs            : [${${UTESTNAME}_libs}]")
  coolfluid_log_file("${UTESTNAME}_cflibs          : [${${UTESTNAME}_cflibs}]")
  coolfluid_log_file("${UTESTNAME}_all_mods_pres   : [${${UTESTNAME}_all_mods_pres}]")
  coolfluid_log_file("${UTESTNAME}_requires_mods   : [${${UTESTNAME}_requires_mods}]")
  coolfluid_log_file("${UTESTNAME}_P_SOURCES       : [${${UTESTNAME}_P_SOURCES}]")
  coolfluid_log_file("${UTESTNAME}_LINK_FLAGS      : [${${UTESTNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${UTESTNAME}_TYPE            : [${${UTESTNAME}_TYPE}]")


endmacro( coolfluid_add_unittest )
