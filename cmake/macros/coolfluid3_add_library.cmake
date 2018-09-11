##############################################################################
# macro for adding a library
##############################################################################

macro( coolfluid3_add_library )

    set( options TEST KERNEL PYTHON_MODULE )
    set( single_value_args TARGET TYPE INSTALL_HEADERS LINKER_LANGUAGE )
    set( multi_value_args  SOURCES PLUGINS MOC RCC LIBS INCLUDES DEPENDS DEFINITIONS CFLAGS CXXFLAGS FFLAGS CONDITION )

    cmake_parse_arguments( _PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN} )

    if(_PAR_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unknown keywords given to coolfluid3_add_library(): \"${_PAR_UNPARSED_ARGUMENTS}\"")
    endif()

    if( NOT _PAR_TARGET  )
      message(FATAL_ERROR "The call to coolfluid3_add_library() doesn't specify the TARGET.")
    endif()

    if( NOT _PAR_SOURCES )
      message(FATAL_ERROR "The call to coolfluid3_add_library() doesn't specify the SOURCES.")
    endif()

    ### some generic setup

    set( LIBNAME ${_PAR_TARGET} )
    string( TOUPPER ${LIBNAME} LIBNAME_CAPS )

    set( ${LIBNAME}_is_coolfluid_library TRUE CACHE INTERNAL "" )

    # option to build it or not (option is advanced and does not appear in the cmake gui)
    option( CF3_BUILD_${LIBNAME} "Build the ${LIBNAME} library" ON )
    mark_as_advanced( CF3_BUILD_${LIBNAME} )

    get_filename_component( DIRNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME )

    # check if all required plugins are present
    set( ${LIBNAME}_has_all_plugins 1 )
    if( _PAR_PLUGINS )
        foreach( req_plugin ${_PAR_PLUGINS} )
            list( FIND CF3_PLUGIN_LIST ${req_plugin} pos )
            if( ${pos} EQUAL -1 )
                set( ${LIBNAME}_has_all_plugins 0 )
                if( CF3_BUILD_${LIBNAME} )
                    coolfluid_log_verbose( "\# LIB [${LIBNAME}] requires plugin [${req_plugin}] which is not present")
                endif()
            endif()
        endforeach()
    endif()

    set( ${LIBNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )
    set( ${LIBNAME}_includedirs "" )
    ### sources

    # separate the source files and remove them from the orphan list
    coolfluid_separate_sources( "${_PAR_SOURCES}" ${LIBNAME} )

    source_group( Headers FILES ${${LIBNAME}_headers} )
    source_group( Sources FILES ${${LIBNAME}_sources} )

    ### conditional build

    if( DEFINED _PAR_CONDITION )
        set(_target_condition_file "${CMAKE_CURRENT_BINARY_DIR}/set_${_PAR_TARGET}_condition.cmake")
        file( WRITE  ${_target_condition_file} "  if( ")
        foreach( term ${_PAR_CONDITION} )
            file( APPEND ${_target_condition_file} " ${term}")
        endforeach()
        file( APPEND ${_target_condition_file} " )\n    set(_${_PAR_TARGET}_condition TRUE)\n  else()\n    set(_${_PAR_TARGET}_condition FALSE)\n  endif()\n")
        include( ${_target_condition_file} )
    else()
        set( _${_PAR_TARGET}_condition TRUE )
    endif()

    if( CF3_BUILD_${LIBNAME} AND ${LIBNAME}_has_all_plugins AND _${_PAR_TARGET}_condition )
        set( ${LIBNAME}_builds YES CACHE INTERNAL "" FORCE )
    else()
        set( ${LIBNAME}_builds NO  CACHE INTERNAL "" FORCE )
    endif()

    if( ${LIBNAME}_builds )

        coolfluid_log_file( " +++ LIB   [${LIBNAME}]" )

        # kernel library (?)
        if( _PAR_KERNEL )
            set( CF3_KERNEL_LIBS ${CF3_KERNEL_LIBS} ${LIBNAME} CACHE INTERNAL "" )
        endif()

        # defines the type of library
        if( DEFINED _PAR_TYPE )
            # checks that is either SHARED or STATIC or MODULE
            if( NOT _PAR_TYPE MATCHES "STATIC" AND
                NOT _PAR_TYPE MATCHES "SHARED" AND
                NOT _PAR_TYPE MATCHES "MODULE" )
                message( FATAL_ERROR "library type must be one of [ STATIC | SHARED | MODULE ]" )
            endif()
        endif()

        # include dirs from plugins
        foreach( plugin ${_PAR_PLUGINS} )
            list( APPEND ${LIBNAME}_includedirs ${${plugin}_DIR} )
        endforeach()

        # include dirs from INCLUDES parameter
        if( _PAR_INCLUDES )
          foreach( path ${_PAR_INCLUDES} )
            list( APPEND ${LIBNAME}_includedirs ${path} )
          endforeach()
        endif()

        # add include dirs of LIBS if defined
        foreach( lib ${_PAR_LIBS} )
            foreach( path ${${lib}_includedirs} ) # skip NOTFOUND
                list( APPEND ${LIBNAME}_includedirs ${path} )
            endforeach()
        endforeach()

        # add include dirs if defined
        foreach( path ${${LIBNAME}_includedirs} ) # skip NOTFOUND
            if( path )
                include_directories( ${path} )
            else()
                message( WARNING "Path ${path} was skipped" )
            endif()
        endforeach()

        # create the moc files
        #    -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED -> fixes compilation of MOC fileswhen Boost 1.48 is in use
        if( QT4_FOUND AND DEFINED _PAR_MOC )
          qt4_wrap_cpp(_gen_MOC ${_PAR_MOC} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED )
        endif()

        # add the library target
        add_library(${_PAR_TARGET} ${_PAR_TYPE} ${_PAR_SOURCES} ${_gen_MOC} ${_PAR_RCC})

        set_target_properties( ${_PAR_TARGET} PROPERTIES LINK_FLAGS "${CF3_LIBRARY_LINK_FLAGS}" )
        set_target_properties( ${_PAR_TARGET} PROPERTIES DEFINE_SYMBOL ${LIBNAME_CAPS}_EXPORTS )

        # add extra dependencies
        if( DEFINED _PAR_DEPENDS)
          add_dependencies( ${_PAR_TARGET} ${_PAR_DEPENDS} )
        endif()

        # add the link libraries
        if( DEFINED _PAR_LIBS )
          list(REMOVE_ITEM _PAR_LIBS debug)
          list(REMOVE_ITEM _PAR_LIBS optimized)
          foreach( lib ${_PAR_LIBS} ) # skip NOTFOUND
            if( lib )
              target_link_libraries( ${_PAR_TARGET} ${lib} )
            else()
              message( WARNING "Lib ${lib} was skipped" )
            endif()
          endforeach()
        endif()

        # if mpi was found add it to the libraries
        if( CF3_HAVE_MPI AND NOT CF3_USES_MPI_COMPILER )
            target_link_libraries( ${LIBNAME} ${MPI_CXX_LIBRARIES} )
        endif()

        # python module support
        if( _PAR_PYTHON_MODULE )
          if( APPLE )
            get_target_property(LIB_LOCNAME ${LIBNAME} LOCATION)
            set(DSO_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}${LIB_SUFFIX})
            set(PYTHON_MODULE_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}.so")
            add_custom_command(
              TARGET ${_PAR_TARGET}
              POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E create_symlink ${CF3_DSO_DIR}/${DSO_LIB_NAME} ${CF3_DSO_DIR}/${PYTHON_MODULE_NAME} )
          endif()
        endif()

        # add local flags
        if( DEFINED _PAR_CFLAGS )
            set_source_files_properties( ${${_PAR_TARGET}_c_srcs}   PROPERTIES COMPILE_FLAGS "${_PAR_CFLAGS}" )
        endif()
        if( DEFINED _PAR_CXXFLAGS )
            set_source_files_properties( ${${_PAR_TARGET}_cxx_srcs} PROPERTIES COMPILE_FLAGS "${_PAR_CXXFLAGS}" )
        endif()
        if( DEFINED _PAR_FFLAGS )
            set_source_files_properties( ${${_PAR_TARGET}_f_srcs}   PROPERTIES COMPILE_FLAGS "${_PAR_FFLAGS}" )
        endif()

        if( NOT _PAR_TEST )

            # add installation paths
            install( TARGETS ${LIBNAME}
              RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR} COMPONENT libraries
              LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT libraries
              ARCHIVE DESTINATION ${CF3_INSTALL_LIB_DIR} COMPONENT libraries
            )

            # install headers

            set( _header_destination "include/${PROJECT_NAME}/${DIRNAME}" )

            if( DEFINED _PAR_INSTALL_HEADERS )
                  install( FILES ${${_PAR_TARGET}_headers} }
                           DESTINATION ${_header_destination}
                           COMPONENT headers )
            endif()

        endif()

        # add definitions to compilation
        if( DEFINED _PAR_DEFINITIONS )
            get_property( _target_defs TARGET ${_PAR_TARGET} PROPERTY COMPILE_DEFINITIONS )
            list( APPEND _target_defs ${_PAR_DEFINITIONS} )
            set_property( TARGET ${_PAR_TARGET} PROPERTY COMPILE_DEFINITIONS ${_target_defs} )
        endif()

        # set linker language
        if( DEFINED _PAR_LINKER_LANGUAGE )
            set_property( TARGET ${_PAR_TARGET} PROPERTY LINKER_LANGUAGE ${_PAR_LINKER_LANGUAGE} )
        endif()

        get_target_property( ${LIBNAME}_LINK_LIBRARIES  ${LIBNAME} LINK_LIBRARIES )
    endif()

  set( ${LIBNAME}_includedirs ${${LIBNAME}_includedirs} CACHE INTERNAL "" )

  # log some info about the library


  coolfluid_log_file("${LIBNAME} user option     : [${CF3_BUILD_${LIBNAME}}]")
  coolfluid_log_file("${LIBNAME}_builds          : [${${LIBNAME}_builds}]")
  coolfluid_log_file("${LIBNAME}_dir             : [${${LIBNAME}_dir}]")
  coolfluid_log_file("${LIBNAME} kernel lib      : [${_PAR_KERNEL}]")
  coolfluid_log_file("${LIBNAME}_includedirs     : [${${LIBNAME}_includedirs}]")
  coolfluid_log_file("${LIBNAME}_libs            : [${_PAR_LIBS}]")
  coolfluid_log_file("${LIBNAME}_has_all_plugins : [${${LIBNAME}_has_all_plugins}]")
  coolfluid_log_file("${LIBNAME}_requires_plugins: [${_PAR_PLUGINS}]")
  coolfluid_log_file("${LIBNAME}_sources         : [${_PAR_SOURCES}]")
  coolfluid_log_file("${LIBNAME}_LINK_LIBRARIES  : [${${LIBNAME}_LINK_LIBRARIES}]")

  coolfluid_log_file("${LIBNAME} install dir     : [${${LIBNAME}_INSTALL_HEADERS}]")
  coolfluid_log_file("${LIBNAME} install API     : [${CF3_BUILD_${LIBNAME}_API}]")


endmacro( coolfluid3_add_library  )


