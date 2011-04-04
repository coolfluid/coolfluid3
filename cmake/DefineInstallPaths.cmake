# recreate the coolfluid_log_file

set( PROJECT_LOG_FILE ${PROJECT_BINARY_DIR}/CMakeLogInfo.txt )
file( WRITE ${PROJECT_LOG_FILE} "coolfluid cmake log file\n")

# set installation paths

set( CF_INSTALL_ROOT_DIR     coolfluid-${CF_KERNEL_VERSION}             CACHE STRING "Installation root directory path")
set( CF_INSTALL_BIN_DIR      ${CF_INSTALL_ROOT_DIR}/bin                 CACHE STRING "Installation path for application binaries" )
set( CF_INSTALL_LIB_DIR      ${CF_INSTALL_ROOT_DIR}/lib                 CACHE STRING "Installation path for libraries" )
set( CF_INSTALL_INCLUDE_DIR  ${CF_INSTALL_ROOT_DIR}/include/coolfluid   CACHE STRING "Installation path for API header files" )
set( CF_INSTALL_SHARE_DIR    ${CF_INSTALL_ROOT_DIR}/share/coolfluid     CACHE STRING "Installation path for shared files" )
set( CF_INSTALL_ARCHIVE_DIR  ${CF_INSTALL_ROOT_DIR}/archive             CACHE STRING "Installation path for archive files")

mark_as_advanced( CF_INSTALL_BIN_DIR CF_INSTALL_LIB_DIR CF_INSTALL_INCLUDE_DIR CF_INSTALL_SHARE_DIR)

# setup library building rpaths

set( CMAKE_SKIP_BUILD_RPATH              FALSE ) # use RPATHs for the build tree
set( CMAKE_BUILD_WITH_INSTALL_RPATH      FALSE ) # use build RPATH and when install substitute with install path
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH   TRUE  ) # add the automatic parts to RPATH which point to dirs outside build tree

set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CF_INSTALL_LIB_DIR}" )

# create the dso directory for shared libraries

set( coolfluid_DSO_DIR ${coolfluid_BINARY_DIR}/dso )

file( MAKE_DIRECTORY ${coolfluid_DSO_DIR} )
