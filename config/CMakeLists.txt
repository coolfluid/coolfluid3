# file for general configuration for the whole framework
#
# config options that affect all the framework libraries and components
# must be handled in this directory

string( TOUPPER "${CMAKE_BUILD_TYPE}" CF3_BUILD_TYPE )

# coolfluid generic settings
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid-config.hpp.in ${coolfluid_BINARY_DIR}/coolfluid-config.hpp)

# coolfluid packages found
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid-packages.hpp.in ${coolfluid_BINARY_DIR}/coolfluid-packages.hpp)

# coolfluid profiling configs
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid-profiling.hpp.in ${coolfluid_BINARY_DIR}/coolfluid-profiling.hpp)

# Some hard-coded paths
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/coolfluid-paths.hpp.in ${coolfluid_BINARY_DIR}/coolfluid-paths.hpp)

# install global configuration headers
install ( FILES ${coolfluid_BINARY_DIR}/coolfluid-config.hpp
                ${coolfluid_BINARY_DIR}/coolfluid-packages.hpp
                ${coolfluid_BINARY_DIR}/coolfluid-profiling.hpp
          DESTINATION
                ${CF3_INSTALL_INCLUDE_DIR} )


