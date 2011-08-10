# find boost package (essential)
# set( Boost_DEBUG 1 )  # to debug boost search
set( Boost_USE_STATIC_LIBS ${CF_ENABLE_STATIC} )
set( Boost_USE_MULTITHREAD ON  )
# find based on minimal version defined below
set( CF_Boost_MINIMAL_VERSION "1.46.1" )

# components to search for
list( APPEND CF_Boost_COMPONENTS thread iostreams filesystem system regex unit_test_framework date_time program_options )
if(CF_ENABLE_PYTHON)
  list( APPEND CF_Boost_COMPONENTS python )
endif()

find_package( Boost ${CF_Boost_MINIMAL_VERSION} COMPONENTS ${CF_Boost_COMPONENTS} QUIET )

coolfluid_log_file( "Boost version      [${Boost_LIB_VERSION}]" )
coolfluid_log_file( "Boost include path [${Boost_INCLUDE_DIR}]" )
coolfluid_log_file( "Boost libraries    [${Boost_LIBRARIES}]"   )

add_definitions( -DBOOST_ENABLE_ASSERT_HANDLER )

# if not found give more information
if( NOT Boost_FOUND )
  coolfluid_log( ${Boost_ERROR_REASON} )
  message( FATAL_ERROR "Boost is required to compile coolfluid Kernel" )
endif()

# add boost include path
include_directories( ${Boost_INCLUDE_DIR} )

# add boost libraries to list of third party libraries
list( APPEND CF_DEPS_LIBRARIES ${Boost_LIBRARIES} )

# filter out the unit test libs from the boost libraries
# only unit testslink to this
set(CF_BOOST_LIBRARIES "" )
foreach( blib ${Boost_LIBRARIES} )
  if( NOT ${blib} MATCHES "[a-zA-Z0-9]*unit_test_framework[a-zA-Z0-9]*" )
    list( APPEND CF_BOOST_LIBRARIES ${blib} )
  endif()
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
# CF_HAVE_BOOST_ERFC )

