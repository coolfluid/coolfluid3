##############################################################################
# macro for adding a testing application in the project
##############################################################################
MACRO( CF_ADD_UNITTEST UTESTNAME )

  # option to build it or not
  OPTION ( CF_BUILD_${UTESTNAME} "Build the ${UTESTNAME} testing application" ON )

  # add to list of local apps
  LIST ( APPEND CF_LOCAL_UTESTNAMES ${UTESTNAME} )

#   CF_DEBUG_VAR(CF_MODULES_LIST)

  # check if all required modules are present
  SET ( ${UTESTNAME}_all_mods_pres ON )
  FOREACH ( reqmod ${${UTESTNAME}_requires_mods} )
    LIST ( FIND CF_MODULES_LIST ${reqmod} pos )
    IF ( ${pos} EQUAL -1 )
      SET ( ${UTESTNAME}_all_mods_pres OFF )
      IF ( CF_BUILD_${UTESTNAME} )
          LOGVERBOSE ( "     \# testing app [${UTESTNAME}] requires module [${reqmod}] which is not present")
      ENDIF()
    ENDIF()
  ENDFOREACH ( reqmod ${${UTESTNAME}_requires_mods} )

  SET ( ${UTESTNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  IF ( CF_BUILD_${UTESTNAME} AND ${UTESTNAME}_all_mods_pres)
    SET ( ${UTESTNAME}_will_compile ON )
  ELSE()
    SET ( ${UTESTNAME}_will_compile OFF )
  ENDIF()

  LOGVERBOSE ("test_${UTESTNAME} = ${${UTESTNAME}_will_compile}")

  # compile if selected and all required modules are present
  IF (${UTESTNAME}_will_compile)

    IF( DEFINED ${UTESTNAME}_includedirs )
      INCLUDE_DIRECTORIES(${${UTESTNAME}_includedirs})
    ENDIF()

    CF_SEPARATE_SOURCES("${${UTESTNAME}_files}" ${UTESTNAME})

    SOURCE_GROUP ( Headers FILES ${${UTESTNAME}_headers} )
    SOURCE_GROUP ( Sources FILES ${${UTESTNAME}_sources} )

    LOG ( " +++ TEST [${UTESTNAME}]" )

    ADD_EXECUTABLE ( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers} )

    IF(CF_INSTALL_TESTS)
      # add installation paths
      INSTALL( TARGETS ${UTESTNAME}
        RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
        )
    ENDIF(CF_INSTALL_TESTS)

    # if mpi was found add it to the libraries
    IF   (CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)
#           MESSAGE ( STATUS "${UTESTNAME} links to ${MPI_LIBRARIES}" )
          TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${MPI_LIBRARIES} )
    ENDIF(CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)

    # add external dependency libraries if defined
    IF( DEFINED ${UTESTNAME}_libs )
      TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${${UTESTNAME}_libs} )
    ENDIF(DEFINED ${UTESTNAME}_libs)

    # internal dependencies
    IF ( CF_ENABLE_INTERNAL_DEPS )
      IF( DEFINED ${UTESTNAME}_cflibs )
        TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${${UTESTNAME}_cflibs} )
      ENDIF(DEFINED ${UTESTNAME}_cflibs)
    ELSE()
      TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${CF_KERNEL_LIBS} ${CF_KERNEL_STATIC_LIBS} ${CF_Boost_LIBRARIES} )
    ENDIF(CF_ENABLE_INTERNAL_DEPS)

  ADD_TEST( ${UTESTNAME} ${UTESTNAME} )

  ENDIF(${UTESTNAME}_will_compile)

  GET_TARGET_PROPERTY ( ${UTESTNAME}_LINK_LIBRARIES  ${UTESTNAME} LINK_LIBRARIES )

  # log some info about the app
  LOGFILE("${UTESTNAME} : [${CF_BUILD_${UTESTNAME}}]")
  LOGFILE("${UTESTNAME} : [${${UTESTNAME}_will_compile}]")
  LOGFILE("${UTESTNAME}_dir             : [${${UTESTNAME}_dir}]")
  LOGFILE("${UTESTNAME}_includedirs     : [${${UTESTNAME}_includedirs}]")
  LOGFILE("${UTESTNAME}_libs            : [${${UTESTNAME}_libs}]")
  LOGFILE("${UTESTNAME}_all_mods_pres   : [${${UTESTNAME}_all_mods_pres}]")
  LOGFILE("${UTESTNAME}_requires_mods   : [${${UTESTNAME}_requires_mods}]")
  LOGFILE("${UTESTNAME}_sources         : [${${UTESTNAME}_sources}]")
  LOGFILE("${UTESTNAME}_LINK_LIBRARIES  : [${${UTESTNAME}_LINK_LIBRARIES}]")


ENDMACRO( CF_ADD_APP )
##############################################################################
