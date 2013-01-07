# recreate the coolfluid_log_file

set( PROJECT_LOG_FILE ${PROJECT_BINARY_DIR}/CMakeLogInfo.txt )
file( WRITE ${PROJECT_LOG_FILE} "coolfluid cmake log file\n")

# set installation paths

set( CF3_INSTALL_ROOT_DIR     coolfluid-${CF3_KERNEL_VERSION}             CACHE STRING "Installation root directory path")
set( CF3_INSTALL_BIN_DIR      ${CF3_INSTALL_ROOT_DIR}/bin                 CACHE STRING "Installation path for application binaries" )
set( CF3_INSTALL_LIB_DIR      ${CF3_INSTALL_ROOT_DIR}/lib                 CACHE STRING "Installation path for libraries" )
set( CF3_INSTALL_INCLUDE_DIR  ${CF3_INSTALL_ROOT_DIR}/include/coolfluid   CACHE STRING "Installation path for API header files" )
set( CF3_INSTALL_SHARE_DIR    ${CF3_INSTALL_ROOT_DIR}/share/coolfluid     CACHE STRING "Installation path for shared files" )
set( CF3_INSTALL_ARCHIVE_DIR  ${CF3_INSTALL_ROOT_DIR}/archive             CACHE STRING "Installation path for archive files")

mark_as_advanced( CF3_INSTALL_BIN_DIR CF3_INSTALL_LIB_DIR CF3_INSTALL_INCLUDE_DIR CF3_INSTALL_SHARE_DIR)

# setup library building rpaths

set( CMAKE_SKIP_BUILD_RPATH              FALSE ) # use RPATHs for the build tree
set( CMAKE_BUILD_WITH_INSTALL_RPATH      FALSE ) # use build RPATH and when install substitute with install path
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH   TRUE  ) # add the automatic parts to RPATH which point to dirs outside build tree

set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CF3_INSTALL_LIB_DIR}" )

# create the dso directory for shared libraries

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dso ) # should be change to /lib
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

set( CF3_DSO_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

file( MAKE_DIRECTORY ${CF3_DSO_DIR} )
