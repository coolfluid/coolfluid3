# - Try to find OpenCL
# Once done this will define
#
#  OPENCL_FOUND         - system has OpenCL
#  OPENCL_INCLUDE_DIRS  - the OpenCL include directory
#  OPENCL_LIBRARIES     - link these to use OpenCL

option( CF_SKIP_OPENCL "Skip search for OpenCL library" OFF )

if( NOT CF_SKIP_OPENCL )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${DEPS_ROOT}/include )
  coolfluid_add_trial_include_path( ${OPENCL_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{OPENCL_ROOT}/include )

  coolfluid_add_trial_library_path( ${DEPS_ROOT}/lib )
  coolfluid_add_trial_library_path( ${OPENCL_ROOT}/lib )
  coolfluid_add_trial_library_path( $ENV{OPENCL_ROOT}/lib )

    if(WIN32)

       # this section is not tested

        find_path(OPENCL_INCLUDE_DIRS CL/cl.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
        find_path(OPENCL_INCLUDE_DIRS CL/cl.h  )

        string( COMPARE EQUAL CF_OS_BITS "64" OPENCL_64 )
        if(OPENCL_64)
          find_library(OPENCL_LIBRARIES opencl64 ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
          find_library(OPENCL_LIBRARIES opencl64 )
        else()
          find_library(OPENCL_LIBRARIES opencl32 ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
          find_library(OPENCL_LIBRARIES opencl32 )
        endif()

    endif()

    if(UNIX)

      if(APPLE)

          find_path(OPENCL_INCLUDE_DIRS OpenCL/cl.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
          find_path(OPENCL_INCLUDE_DIRS OpenCL/cl.h  )

          find_library(OPENCL_LIBRARIES OpenCL ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
          find_library(OPENCL_LIBRARIES OpenCL )

        else() # Other UNIX, like Linux, etc

          coolfluid_add_trial_include_path( /usr/local/cuda/include )
          coolfluid_add_trial_library_path( /usr/local/cuda/lib )

          find_path(OPENCL_INCLUDE_DIRS CL/cl.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
          find_path(OPENCL_INCLUDE_DIRS CL/cl.h  )

          find_library(OPENCL_LIBRARIES OpenCL ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
          find_library(OPENCL_LIBRARIES OpenCL  )

      endif()

    endif()

endif()

set( OPENCL_FOUND "FALSE" )
if( OPENCL_LIBRARIES AND OPENCL_INCLUDE_DIRS )
    set( OPENCL_FOUND "TRUE" )
endif()

mark_as_advanced( OPENCL_INCLUDE_DIRS OPENCL_LIBRARIES )

coolfluid_log_file( "OPENCL_FOUND: [${OPENCL_FOUND}]" )
if(OPENCL_FOUND)
  coolfluid_log_file( "  OPENCL_INCLUDE_DIRS: [${OPENCL_INCLUDE_DIRS}]" )
  coolfluid_log_file( "  OPENCL_LIBRARIES:    [${OPENCL_LIBRARIES}]" )
endif()
