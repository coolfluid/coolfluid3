### prints the build summary

# get some variables
site_name(CF3_BUILD_HOSTNAME)

coolfluid_get_date(CF3_BUILD_DATE)

get_property( CF3_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES )

set( print_counter 0 )

# print the header with versions and operating system

coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( " coolfluid configuration summary " )
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " version & kernel      : [${CF3_VERSION}] @ [${CF3_KERNEL_VERSION}]" )
if(coolfluid_git_revision_sha)
  coolfluid_log( " coolfluid revision    : [${coolfluid_git_revision_sha}] @ [${coolfluid_git_revision_date}]" )
endif()
coolfluid_log( " hostname & date       : [${CF3_BUILD_HOSTNAME}] @ [${CF3_BUILD_DATE}]" )
coolfluid_log( " operating system      : [${CMAKE_SYSTEM}] @ [${CF3_OS_BITS}] bits" )

# print cmake information

coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " cmake version         : [${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}]" )
coolfluid_log( " cmake generator       : [${CMAKE_GENERATOR}]" )
coolfluid_log( " build type            : [${CMAKE_BUILD_TYPE}]" )
coolfluid_log( " boost version         : [${Boost_LIB_VERSION}]" )

# print the compiler information ( put to the log file )

coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " enabled languages     : [${CF3_LANGUAGES}]")
foreach( lang ${CF3_LANGUAGES} )
  set(_spaces "          ")
  if(${lang} STREQUAL "CXX")
    set(_spaces "        ")
  elseif(${lang} STREQUAL "Fortran")
    set(_spaces "    ")
  endif()
  coolfluid_log( " ${lang} compiler ${_spaces} : [${CMAKE_${lang}_COMPILER}]" )
  coolfluid_log_file( " ${lang}  flags         : [${CMAKE_${lang}_FLAGS} ${CMAKE_${lang}_FLAGS_${CMAKE_BUILD_TYPE}}]" )
  coolfluid_log_file( " ${lang}  link flags    : [${CMAKE_CXX_LINK_FLAGS}]" )
endforeach()

coolfluid_log( " compiler id & version : [${CMAKE_CXX_COMPILER_ID} ${CF3_CXX_COMPILER_VERSION}]" )
coolfluid_log_file( " common linker flags   : [${LINK_FLAGS}]" )
coolfluid_log_file( " shared linker flags   : [${CMAKE_SHARED_LINKER_FLAGS}]" )
coolfluid_log_file( " link libraries        : [${LINK_LIBRARIES}]" )


# print most important build options

coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " Documentation         : [${CF3_ENABLE_DOCS}]")
coolfluid_log( " Unit Tests            : [${CF3_ENABLE_UNIT_TESTS}]")
coolfluid_log( " Acceptance Tests      : [${CF3_ENABLE_ACCEPTANCE_TESTS}]")
coolfluid_log( " Performance Tests     : [${CF3_ENABLE_PERFORMANCE_TESTS}]")
coolfluid_log( " GUI                   : [${CF3_HAVE_GUI}]")
coolfluid_log( " Python                : [${CF3_HAVE_PYTHON}]")
coolfluid_log( " Sandbox               : [${CF3_ENABLE_SANDBOX}]")
coolfluid_log( " Code coverage         : [${CF3_ENABLE_CODECOVERAGE}]")
coolfluid_log_file( " Explicit Templates    : [${CF3_HAVE_CXX_EXPLICIT_TEMPLATES}]")

# print install path

coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " install path          : [${CMAKE_INSTALL_PREFIX}]" )
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( "")

# list the active kernel libraries

set( list_kernel_libs "" )
set( print_counter 0 )
foreach( klib ${CF3_KERNEL_LIBS} )

  string(REGEX REPLACE "coolfluid_(.+)" "\\1" klib ${klib} )

  set( list_kernel_libs "${list_kernel_libs} ${klib}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 5 )
    set( print_counter 0 )
    set( list_kernel_libs "${list_kernel_libs}\n\t\t " )
  endif()

endforeach()
coolfluid_log( " Kernel libs: ${list_kernel_libs}")
coolfluid_log( "")


# list the active plugins

set( list_plugins "" )
set( print_counter 0 )
foreach( plugin ${CF3_PLUGIN_LIST} )

  set( list_plugins "${list_plugins} ${plugin}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 5 )
    set( print_counter 0 )
    set( list_plugins "${list_plugins}\n\t\t " )
  endif()

endforeach()
coolfluid_log( " Plugins:     ${list_plugins}")
coolfluid_log( "")

# list the active features

if( COMMAND feature_summary )

    # In this case the packages are not shown in enabled features...
    # Resort to a full summary of enabled features and packages
#    feature_summary( WHAT ENABLED_FEATURES
#                     DESCRIPTION " Enabled Features:"
#                     VAR CF3_ENABLED_FEATURES )
#    coolfluid_log( "${CF3_ENABLED_FEATURES}" )

    # In this case the packages are added inside the enabled features...
    # The summary is made by ourself, and is shorter.
    set( list_features "" )
    set( print_counter 0 )
    get_property( CF3_ENABLED_FEATURES  GLOBAL PROPERTY ENABLED_FEATURES )
    get_property( CF3_PACKAGES_FOUND  GLOBAL PROPERTY PACKAGES_FOUND )
    list( APPEND CF3_ENABLED_FEATURES ${CF3_PACKAGES_FOUND})
    list( REMOVE_DUPLICATES CF3_ENABLED_FEATURES)

    foreach( feature ${CF3_ENABLED_FEATURES})

      get_property(_is_quiet GLOBAL PROPERTY _CMAKE_${feature}_QUIET)
      if(NOT _is_quiet)

        set( list_features "${list_features} ${feature}")

        # break line if necessary
        math( EXPR print_counter '${print_counter}+1'  )
        if( print_counter GREATER 5 )
          set( print_counter 0 )
          set( list_features "${list_features}\n\t\t " )
        endif()
      endif(NOT _is_quiet)
    endforeach()

    coolfluid_log( " Features:    ${list_features}")
    coolfluid_log( "")

    feature_summary(WHAT ALL
                    FILENAME ${PROJECT_LOG_FILE} APPEND)
endif()

foreach( utest ${CF3_ENABLED_UTESTS} )

  set( list_enabled_utests "${list_enabled_utests} ${utest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_enabled_utests "${list_enabled_utests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Enabled unit tests:\n\t\t${list_enabled_utests}" )
if( DEFINED list_enabled_utests )
  coolfluid_log_file( "" )
endif()
foreach( utest ${CF3_DISABLED_UTESTS} )

  set( list_disabled_utests "${list_disabled_utests} ${utest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_disabled_utests "${list_disabled_utests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Disabled unit tests:\n\t\t${list_disabled_utests}" )
if( DEFINED list_disabled_utests )
  coolfluid_log_file( "" )
endif()


foreach( atest ${CF3_ENABLED_ATESTS} )

  set( list_enabled_atests "${list_enabled_atests} ${atest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_enabled_atests "${list_enabled_atests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Enabled acceptance tests:\n\t\t${list_enabled_atests}" )
if( DEFINED list_enabled_atests )
  coolfluid_log_file( "" )
endif()

foreach( atest ${CF3_DISABLED_ATESTS} )

  set( list_disabled_atests "${list_disabled_atests} ${atest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_disabled_atests "${list_disabled_atests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Disabled acceptance tests:\n\t\t${list_disabled_atests}" )
if( DEFINED list_disabled_atests )
  coolfluid_log_file( "" )
endif()

foreach( atest ${CF3_ENABLED_PTESTS} )

  set( list_enabled_ptests "${list_enabled_ptests} ${ptest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_enabled_ptests "${list_enabled_ptests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Enabled performance tests:\n\t\t${list_enabled_ptests}" )
if( DEFINED list_enabled_ptests )
  coolfluid_log_file( "" )
endif()

foreach( ptest ${CF3_DISABLED_PTESTS} )

  set( list_disabled_ptests "${list_disabled_ptests} ${ptest}" )

  # break line if necessary
  math( EXPR print_counter '${print_counter}+1'  )
  if( print_counter GREATER 3 )
    set( print_counter 0 )
    set( list_disabled_ptests "${list_disabled_ptests}\n\t\t" )
  endif()

endforeach()

coolfluid_log_file( " Disabled performance tests:\n\t\t${list_disabled_ptests}" )
if( DEFINED list_disabled_ptests )
  coolfluid_log_file( "" )
endif()

# warn if this is a static build
# TODO: test static building

if(CF3_ENABLE_STATIC)
  coolfluid_log( ">>>> ------------------------------" )
  coolfluid_log( ">>>> IMPORTANT -- STATIC BUILD <<<<" )
  coolfluid_log( ">>>> ------------------------------" )
endif()

