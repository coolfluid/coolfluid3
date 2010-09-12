# file for general configuration for the whole framework
#
# config options that affect all the framework libraries and components
# must be handled in this directory

string( TOUPPER "${CMAKE_BUILD_TYPE}" CF_BUILD_TYPE )

# coolfluid_config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid_config.h.in ${coolfluid_BINARY_DIR}/coolfluid_config.h)

# create the header with the svn version
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid_svnversion.h.in ${coolfluid_BINARY_DIR}/coolfluid_svnversion.h)

# coolfluid_profiling_config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid_profiling_config.h.in ${coolfluid_BINARY_DIR}/coolfluid_profiling_config.h)

# install global configuration headers
install ( FILES ${coolfluid_BINARY_DIR}/coolfluid_config.h
                ${coolfluid_BINARY_DIR}/coolfluid_svnversion.h
          DESTINATION
                ${CF_INSTALL_INCLUDE_DIR} )


