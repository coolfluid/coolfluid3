# recreate the logfile
SET  ( PROJECT_LOG_FILE ${PROJECT_BINARY_DIR}/CMakeLogInfo.txt )
FILE ( WRITE ${PROJECT_LOG_FILE} "coolfluid cmake log file\n")

# set installation paths
set ( CF_INSTALL_BIN_DIR      bin                               CACHE STRING "Installation path for application binaries" )
set ( CF_INSTALL_LIB_DIR      lib                               CACHE STRING "Installation path for libraries" )
set ( CF_INSTALL_INCLUDE_DIR  include/coolfluid-${CF_VERSION}   CACHE STRING "Installation path for API header files" )
set ( CF_INSTALL_SHARE_DIR    share/coolfluid-${CF_VERSION}     CACHE STRING "Installation path for shared files" )

# setup library building rpaths
SET ( CMAKE_SKIP_BUILD_RPATH  FALSE )          # use RPATHs for the build tree 
SET ( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )   # usw build RPATH and when install substitute with install path
SET ( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE ) # add the automatic parts to RPATH which point to dirs outside build tree 
SET ( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CF_INSTALL_LIB_DIR}" ) 

# create the dso directory for shared libraries
SET  ( coolfluid_DSO_DIR ${coolfluid_BINARY_DIR}/dso )
FILE ( MAKE_DIRECTORY ${coolfluid_DSO_DIR} )


