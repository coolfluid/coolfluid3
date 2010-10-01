##############################################################################
# macro for adding a library in the project
##############################################################################
macro( coolfluid_add_library LIBNAME )

  # option to build it or not
  option( CF_BUILD_${LIBNAME} "Build the ${LIBNAME} library" ON )
  mark_as_advanced( CF_BUILD_${LIBNAME}_API )	# and mark the option advanced

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

  # add to list of local libs
  list( APPEND CF_LOCAL_LIBNAMES ${LIBNAME} )

  # do we still need this in CF3???

  # check if all required modules are present
  set( ${LIBNAME}_all_mods_pres ON )
  foreach( reqmod ${${LIBNAME}_requires_mods} )
    list( FIND CF_MODULES_LIST ${reqmod} pos )
    if( ${pos} EQUAL -1 )
      set( ${LIBNAME}_all_mods_pres OFF )
      if( CF_BUILD_${LIBNAME} )
          coolfluid_log_verbose( "\# lib [${LIBNAME}] requires module [${reqmod}] which is not present")
      endif()
    endif()
  endforeach()

  if(CF_BUILD_${LIBNAME} AND ${LIBNAME}_all_mods_pres)
    set( ${LIBNAME}_will_compile ON )
  else()
    set( ${LIBNAME}_will_compile OFF )
  endif()

  set( ${LIBNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )
  set( CF_COMPILES_${LIBNAME} ${${LIBNAME}_will_compile} CACHE INTERNAL "" FORCE )

  coolfluid_log_verbose("lib_${LIBNAME} = ${${LIBNAME}_will_compile}")

  # separate the source files
  # and remove them from the orphan list

  coolfluid_separate_sources("${${LIBNAME}_files}" ${LIBNAME})

  source_group( Headers FILES ${${LIBNAME}_headers} )
  source_group( Sources FILES ${${LIBNAME}_sources} )

  # compile if selected and all required modules are present
  if(${LIBNAME}_will_compile)

    # if is kernel library
    # add the library to the list of kernel libs
    # and add option to install or not the API headers
    # default for kernel libs is to install
    # default for plugin libs is not to install
    if( ${LIBNAME}_kernellib )
        option( CF_BUILD_${LIBNAME}_API "Publish the ${LIBNAME} (kernel) library API" ON )
        coolfluid_append_cached_list( CF_KERNEL_LIBS ${LIBNAME} )
    else()
        option( CF_BUILD_${LIBNAME}_API "Publish the ${LIBNAME} (plugin) library API" OFF )
    endif()
    mark_as_advanced( CF_BUILD_${LIBNAME}_API )	# and mark the option advanced


    # add include dirs if defined
    if( DEFINED ${LIBNAME}_includedirs )
      INCLUDE_DIRECTORIES(${${LIBNAME}_includedirs})
    endif()

    coolfluid_log( " +++ LIB [${LIBNAME}]" )

    ADD_LIBRARY(${LIBNAME} ${${LIBNAME}_buildtype} ${${LIBNAME}_sources} ${${LIBNAME}_headers}  ${${LIBNAME}_moc_files} ${${LIBNAME}_RCC})

    SET_TARGET_PROPERTIES( ${LIBNAME} PROPERTIES LINK_FLAGS "${CF_LIBRARY_LINK_FLAGS}" )
    SET_TARGET_PROPERTIES( ${LIBNAME} PROPERTIES DEFINE_SYMBOL ${LIBNAME}_EXPORTS )

    # add installation paths
    INSTALL ( TARGETS ${LIBNAME}
      RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
      LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
      ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
    )

    # install headers for the libraries but
    # check if this library headers should be installed with the API
    if( CF_BUILD_${LIBNAME}_API )
      # replace the current directory with target
      string( REPLACE ${CMAKE_BINARY_DIR} ${CF_INSTALL_INCLUDE_DIR} ${LIBNAME}_INSTALL_HEADERS ${CMAKE_CURRENT_BINARY_DIR} )
      string( REPLACE coolfluid/src  coolfluid ${LIBNAME}_INSTALL_HEADERS ${${LIBNAME}_INSTALL_HEADERS} )

      install( FILES ${${LIBNAME}_headers} DESTINATION ${${LIBNAME}_INSTALL_HEADERS})

      coolfluid_log_file("${LIBNAME}_INSTALL_HEADERS : [${${LIBNAME}_INSTALL_HEADERS}]")
    endif()

    # add coolfluid internal dependency libraries if defined
    if( DEFINED ${LIBNAME}_cflibs )
        # message( STATUS "${LIBNAME} has ${${LIBNAME}_cflibs}}" )
        TARGET_LINK_LIBRARIES ( ${LIBNAME} ${${LIBNAME}_cflibs} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${LIBNAME}_libs )
      #	message( STATUS "${LIBNAME} has ${${LIBNAME}_libs}}" )
      TARGET_LINK_LIBRARIES( ${LIBNAME} ${${LIBNAME}_libs} )
    endif()

    # if mpi was found add it to the libraries
    if(CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)
    #           message( STATUS "${LIBNAME} links to ${MPI_LIBRARIES}" )
        TARGET_LINK_LIBRARIES( ${LIBNAME} ${MPI_LIBRARIES} )
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
		if( NOT ${LIBNAME}_kernellib AND CF_ENABLE_STATIC )
				coolfluid_append_cached_list( CF_KERNEL_STATIC_LIBS ${LIBNAME} )
		endif()

	endif()

	get_target_property( ${LIBNAME}_LINK_LIBRARIES  ${LIBNAME} LINK_LIBRARIES )

  # log some info about the library
  coolfluid_log_file("${LIBNAME} enabled         : [${CF_BUILD_${LIBNAME}}]")
  coolfluid_log_file("${LIBNAME} will compile    : [${${LIBNAME}_will_compile}]")
  coolfluid_log_file("${LIBNAME} install dir     : [${${LIBNAME}_INSTALL_HEADERS}]")
  coolfluid_log_file("${LIBNAME} install API     : [${CF_BUILD_${LIBNAME}_API}]")
  coolfluid_log_file("${LIBNAME}_dir             : [${${LIBNAME}_dir}]")
  coolfluid_log_file("${LIBNAME}_kernellib       : [${${LIBNAME}_kernellib}]")
  coolfluid_log_file("${LIBNAME}_includedirs     : [${${LIBNAME}_includedirs}]")
  coolfluid_log_file("${LIBNAME}_libs            : [${${LIBNAME}_libs}]")
  coolfluid_log_file("${LIBNAME}_all_mods_pres   : [${${LIBNAME}_all_mods_pres}]")
  coolfluid_log_file("${LIBNAME}_requires_mods   : [${${LIBNAME}_requires_mods}]")
  coolfluid_log_file("${LIBNAME}_sources         : [${${LIBNAME}_sources}]")
  coolfluid_log_file("${LIBNAME}_LINK_LIBRARIES  : [${${LIBNAME}_LINK_LIBRARIES}]")

endmacro( coolfluid_add_library )
##############################################################################
