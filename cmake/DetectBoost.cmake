# find boost package (essential)
# SET ( Boost_DEBUG 1 )  # to debug boost search
SET ( Boost_USE_STATIC_LIBS ${CF_ENABLE_STATIC} )
SET ( Boost_USE_MULTITHREAD ON  )
# find based on minimal version defined below
SET ( Boost_FIND_VERSION        ON   )
SET ( Boost_FIND_VERSION_MAJOR  "1"  )
SET ( Boost_FIND_VERSION_MINOR  "42" )
SET ( Boost_FIND_VERSION_PATCH  "0"  )
# older cmakes dont have these versions
SET ( Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" )
# components to search for
LIST ( APPEND CF_Boost_COMPONENTS thread filesystem system regex unit_test_framework iostreams )

find_package( Boost COMPONENTS ${CF_Boost_COMPONENTS} )

LOG ( "Boost include path [${Boost_INCLUDE_DIR}]" )
LOG ( "Boost lib version  [${Boost_LIB_VERSION}]" )
LOG ( "Boost libraries    [${Boost_LIBRARIES}]"   )

# if not found give more information
IF ( NOT Boost_FOUND )
  LOG ( ${Boost_ERROR_REASON} )
  MESSAGE ( FATAL_ERROR "Boost is required to compile coolfluid Kernel" )
ENDIF()
# add boost include path
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
SET ( CF_Boost_LIBRARIES ${Boost_LIBRARIES} )
ADD_DEFINITIONS ( -DBOOST_STRICT_CONFIG )
