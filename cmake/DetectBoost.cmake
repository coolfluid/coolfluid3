# find boost package (essential)
#set( Boost_DEBUG 1 )  # to debug boost search
set( Boost_USE_STATIC_LIBS ${CF3_ENABLE_STATIC} )
set( Boost_USE_MULTITHREAD ON  )
# find based on minimal version defined below
set( CF3_Boost_MINIMAL_VERSION "1.46.1" )
set( Boost_ADDITIONAL_VERSIONS "1.47" "1.46" )

#disable looking in system paths
set(Boost_NO_SYSTEM_PATHS ON)

# components to search for
list( APPEND CF3_Boost_CORE_COMPONENTS thread iostreams filesystem system regex date_time program_options )
list( APPEND CF3_Boost_OTHER_COMPONENTS unit_test_framework)
if(CF3_ENABLE_PYTHON)
  list( APPEND CF3_Boost_OPTIONAL_COMPONENTS python )
endif()

find_package( Boost ${CF3_Boost_MINIMAL_VERSION} COMPONENTS ${CF3_Boost_CORE_COMPONENTS} ${CF3_Boost_OTHER_COMPONENTS} ${CF3_Boost_OPTIONAL_COMPONENTS} QUIET )

coolfluid_log_file( "Boost version      [${Boost_LIB_VERSION}]" )
coolfluid_log_file( "Boost include path [${Boost_INCLUDE_DIR}]" )
coolfluid_log_file( "Boost libraries    [${Boost_LIBRARIES}]"   )

add_definitions( -DBOOST_ENABLE_ASSERT_HANDLER )

set(Boost_FOUND TRUE)
foreach( blib IN LISTS CF3_Boost_CORE_COMPONENTS CF3_Boost_OTHER_COMPONENTS)
  string(TOUPPER ${blib} blib_upper)
  if( NOT Boost_${blib_upper}_FOUND )
    set(Boost_FOUND FALSE)
  endif()
endforeach()

# if not found give more information
if( NOT Boost_FOUND )
  coolfluid_log( ${Boost_ERROR_REASON} )
  message( FATAL_ERROR "Boost is required to compile coolfluid Kernel" )
endif()

# add boost include path
include_directories( ${Boost_INCLUDE_DIR} )

# add boost libraries to list of third party libraries
coolfluid_set_package( PACKAGE Boost DESCRIPTION "C++ general productivity library"
                       URL "http://www.boost.org"
                       TYPE REQUIRED
                       VARS Boost_INCLUDE_DIR Boost_LIBRARIES
                       QUIET
                     )

# filter out the unit test libs from the boost libraries
# only unit testslink to this
set(CF3_BOOST_LIBRARIES "" )
foreach( blib IN LISTS CF3_Boost_CORE_COMPONENTS )
  string(TOUPPER ${blib} blib_upper)
  list( APPEND CF3_BOOST_LIBRARIES ${Boost_${blib_upper}_LIBRARY} )
endforeach()
#######################################################################################
# assume boost minimum version has it
#  coolfluid_log( "+++++  Checking for boost erfc function" )
# set( CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIR} )
# check_cxx_source_compiles (
# "#include <boost/math/special_functions/erf.hpp>
# int main(int argc, char* argv[])
# {
#   boost::math::erfc(0.);
# }"
# CF3_HAVE_BOOST_ERFC )

