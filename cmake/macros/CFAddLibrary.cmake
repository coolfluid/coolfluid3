##############################################################################
# macro for adding a library in the project
##############################################################################

macro( coolfluid_add_library LIBNAME )

  # option to build it or not (option is advanced and does not appear in the cmake gui)
  option( CF3_BUILD_${LIBNAME} "Build the ${LIBNAME} library" ON )
  mark_as_advanced( CF3_BUILD_${LIBNAME} )

  # by default libraries are not part of the kernel
  if( NOT DEFINED ${LIBNAME}_kernellib )
    set( ${LIBNAME}_kernellib OFF )
  endif()

  # library is shared or static?
  if( BUILD_SHARED_LIBS )
    set( ${LIBNAME}_buildtype SHARED )
  else()
    set( ${LIBNAME}_buildtype STATIC )
  endif()

  # check if all required plugins are present
  set( ${LIBNAME}_has_all_plugins TRUE )
  foreach( req_plugin ${${LIBNAME}_requires_plugins} )
    list( FIND CF3_PLUGIN_LIST ${req_plugin} pos )
    if( ${pos} EQUAL -1 )
      set( ${LIBNAME}_has_all_plugins FALSE )
      if( CF3_BUILD_${LIBNAME} )
          coolfluid_log_verbose( "\# LIB [${LIBNAME}] requires plugin [${req_plugin}] which is not present")
      endif()
    endif()
  endforeach()

  set( ${LIBNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  # separate the source files and remove them from the orphan list

  coolfluid_separate_sources("${${LIBNAME}_files}" ${LIBNAME})

  source_group( Headers FILES ${${LIBNAME}_headers} )
  source_group( Sources FILES ${${LIBNAME}_sources} )

  # set condition if not set outside, default is TRUE
  if( DEFINED  ${LIBNAME}_condition )
    coolfluid_log_verbose("${LIBNAME} has condition [${${LIBNAME}_condition}]")
  else()
    set( ${LIBNAME}_condition TRUE )
  endif()

  if(CF3_BUILD_${LIBNAME} AND ${LIBNAME}_has_all_plugins AND ${LIBNAME}_condition)
    set( ${LIBNAME}_builds YES CACHE INTERNAL "" )
  else()
    set( ${LIBNAME}_builds NO  CACHE INTERNAL "" )
  endif()

  # compile if selected and all required modules are present
  if( ${LIBNAME}_builds )

    coolfluid_log_file( " +++ LIB   [${LIBNAME}]" )

    # add include dirs if defined
    if( DEFINED ${LIBNAME}_includedirs )
      include_directories(${${LIBNAME}_includedirs})
    endif()

    # if is kernel library
    # add the library to the list of kernel libs
    # and add option to install or not the API headers
    # default for kernel libs is to install
    # default for plugin libs is not to install
    if( ${LIBNAME}_kernellib )
        option( CF3_BUILD_${LIBNAME}_API "Publish the ${LIBNAME} (kernel) library API" ON )
        set( CF3_KERNEL_LIBS ${CF3_KERNEL_LIBS} ${LIBNAME} CACHE INTERNAL "" )
    else()
        option( CF3_BUILD_${LIBNAME}_API "Publish the ${LIBNAME} (plugin) library API" OFF )
    endif()
    mark_as_advanced( CF3_BUILD_${LIBNAME}_API )	# and mark the option advanced

    add_library(${LIBNAME} ${${LIBNAME}_buildtype} ${${LIBNAME}_sources} ${${LIBNAME}_headers}  ${${LIBNAME}_moc_files} ${${LIBNAME}_RCC})

    SET_TARGET_PROPERTIES( ${LIBNAME} PROPERTIES LINK_FLAGS "${CF3_LIBRARY_LINK_FLAGS}" )
    string( TOUPPER ${LIBNAME} LIBNAME_CAPS )
    SET_TARGET_PROPERTIES( ${LIBNAME} PROPERTIES DEFINE_SYMBOL ${LIBNAME_CAPS}_EXPORTS )

    # add installation paths
    install( TARGETS ${LIBNAME}
      RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR} COMPONENT libraries
      LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT libraries
      ARCHIVE DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT libraries
    )

    # install headers for the libraries but
    # check if this library headers should be installed with the API
    if( CF3_BUILD_${LIBNAME}_API )
      # replace the current directory with target
      string( REPLACE ${CMAKE_BINARY_DIR} ${CF3_INSTALL_INCLUDE_DIR} ${LIBNAME}_INSTALL_HEADERS ${CMAKE_CURRENT_BINARY_DIR} )
      string( REPLACE coolfluid/src  coolfluid ${LIBNAME}_INSTALL_HEADERS ${${LIBNAME}_INSTALL_HEADERS} )

      install( FILES ${${LIBNAME}_headers}
        DESTINATION ${${LIBNAME}_INSTALL_HEADERS}
        COMPONENT headers
      )

      coolfluid_log_file("${LIBNAME}_INSTALL_HEADERS : [${${LIBNAME}_INSTALL_HEADERS}]")
    endif()

    # add coolfluid internal dependency libraries if defined
    if( DEFINED ${LIBNAME}_cflibs )
        list(REMOVE_DUPLICATES ${LIBNAME}_cflibs)
        # message( STATUS "${LIBNAME} has ${${LIBNAME}_cflibs}}" )
        TARGET_LINK_LIBRARIES ( ${LIBNAME} ${${LIBNAME}_cflibs} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${LIBNAME}_libs )
      #list(REMOVE_DUPLICATES ${LIBNAME}_libs)
      #	message( STATUS "${LIBNAME} has ${${LIBNAME}_libs}}" )
      target_link_libraries( ${LIBNAME} ${${LIBNAME}_libs} )
    endif()

    # if mpi was found add it to the libraries
    if(CF3_HAVE_MPI AND NOT CF3_HAVE_MPI_COMPILER)
        target_link_libraries( ${LIBNAME} ${MPI_LIBRARIES} )
        if( DEFINED MPI_CXX_LIBRARIES )
             target_link_libraries( ${LIBNAME} ${MPI_CXX_LIBRARIES} )
        endif()
    endif()

    # only add link in dso library if building shared libs
    if(BUILD_SHARED_LIBS)
      get_target_property(LIB_LOCNAME ${LIBNAME} LOCATION)
      set(DSO_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}${LIB_SUFFIX})
      if( UNIX )
        ADD_CUSTOM_COMMAND(
          TARGET ${LIBNAME}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove ${coolfluid_DSO_DIR}/${DSO_LIB_NAME}
          COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_LOCNAME} ${coolfluid_DSO_DIR}/${DSO_LIB_NAME}
        )
        if( DEFINED ${LIBNAME}_PYTHON_MODULE)
          if( APPLE )
            set(PYTHON_MODULE_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}.so")
            ADD_CUSTOM_COMMAND(
              TARGET ${LIBNAME}
              POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E create_symlink ${coolfluid_DSO_DIR}/${DSO_LIB_NAME} ${coolfluid_DSO_DIR}/${PYTHON_MODULE_NAME}
            )
          endif()
        endif()
      else()
        ADD_CUSTOM_COMMAND(
          TARGET ${LIBNAME}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove ${coolfluid_DSO_DIR}/${DSO_LIB_NAME}
          COMMAND ${CMAKE_COMMAND} -E copy ${LIB_LOCNAME} ${coolfluid_DSO_DIR}/${DSO_LIB_NAME}
        )
      endif()
    endif()

    # if not kernel lib and static is set
    # then this lib will be added to the list of kernel libs
    if( NOT ${LIBNAME}_kernellib AND CF3_ENABLE_STATIC )
        coolfluid_append_cached_list( CF3_KERNEL_STATIC_LIBS ${LIBNAME} )
    endif()

  endif()

  get_target_property( ${LIBNAME}_LINK_LIBRARIES  ${LIBNAME} LINK_LIBRARIES )

  # log some info about the library
  coolfluid_log_file("${LIBNAME} user option     : [${CF3_BUILD_${LIBNAME}}]")
  coolfluid_log_file("${LIBNAME}_builds          : [${${LIBNAME}_builds}]")
  coolfluid_log_file("${LIBNAME}_dir             : [${${LIBNAME}_dir}]")
  coolfluid_log_file("${LIBNAME}_kernellib       : [${${LIBNAME}_kernellib}]")
  coolfluid_log_file("${LIBNAME}_includedirs     : [${${LIBNAME}_includedirs}]")
  coolfluid_log_file("${LIBNAME}_libs            : [${${LIBNAME}_libs}]")
  coolfluid_log_file("${LIBNAME}_cflibs          : [${${LIBNAME}_cflibs}]")
  coolfluid_log_file("${LIBNAME}_has_all_plugins : [${${LIBNAME}_has_all_plugins}]")
  coolfluid_log_file("${LIBNAME}_requires_plugins: [${${LIBNAME}_requires_plugins}]")
  coolfluid_log_file("${LIBNAME}_sources         : [${${LIBNAME}_sources}]")
  coolfluid_log_file("${LIBNAME}_LINK_LIBRARIES  : [${${LIBNAME}_LINK_LIBRARIES}]")

  coolfluid_log_file("${LIBNAME} install dir     : [${${LIBNAME}_INSTALL_HEADERS}]")
  coolfluid_log_file("${LIBNAME} install API     : [${CF3_BUILD_${LIBNAME}_API}]")

  #coolfluid_install_targets( ${LIBNAME} )

endmacro( coolfluid_add_library )
##############################################################################
