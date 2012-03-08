# Find ParaView

# Qt is needed to compile ParaView
set(ParaView_FOUND 0)
if( QT_FOUND )
  if( DEFINED ParaView_ROOT )
    set( ParaView_PATH "${ParaView_ROOT}/lib/paraview-")
    set( ParaView_MINIMUM_SUPPORTED_VERSION "3.10" )
    file( GLOB ParaView_LIB_DIRS "${ParaView_PATH}3.[0-9][0-9]" )
    set( ParaView_DIR "" )

    list( FIND ParaView_LIB_DIRS "${ParaView_PATH}${ParaView_MINIMUM_SUPPORTED_VERSION}"
             ParaView_LIB_INDEX )

   # if the supported version was found
   if( NOT ${ParaView_LIB_INDEX} EQUAL -1 )
     list( GET ParaView_LIB_DIRS ${ParaView_LIB_INDEX} ParaView_DIR )
   else() # if not found, we look for the most recent one that is compatible
     list( SORT ParaView_LIB_DIRS )
     list( LENGTH ParaView_LIB_DIRS ParaView_LIB_COUNT )

     # list is ordered so the most recent version is the last item
     math( EXPR ParaView_LIB_INDEX "${ParaView_LIB_COUNT} - 1" )

     # iterate over the found ParaView installations
     while( ${ParaView_LIB_INDEX} GREATER -1 AND "${ParaView_DIR}" STREQUAL "")
       list( GET ParaView_LIB_DIRS ${ParaView_LIB_INDEX} ParaView_TMP_DIR)

       # check that config version script exists
       if( EXISTS ${ParaView_TMP_DIR}/ParaViewConfigVersion.cmake )
         set( PACKAGE_FIND_VERSION ${ParaView_MINIMUM_SUPPORTED_VERSION})
         include(${ParaView_TMP_DIR}/ParaViewConfigVersion.cmake)

         # if this version is compatible with our minimum ersion, it's OK
         if( PACKAGE_VERSION_COMPATIBLE )
           set( ParaView_DIR ${ParaView_TMP_DIR} )
         endif()
       endif()
       math( EXPR ParaView_LIB_INDEX "${ParaView_LIB_INDEX} - 1" ) # decrement index
     endwhile()
   endif()

   if( NOT "${ParaView_DIR}" STREQUAL "" )
     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ParaView_DIR} )
     find_package(ParaView QUIET)
   endif()

   if( NOT ${ParaView_FOUND} )
     coolfluid_log_file("Warning: no compatible ParaView installation could be found under [${ParaView_ROOT}]. The minimum version supported is ${ParaView_MINIMUM_SUPPORTED_VERSION}.")
   else()
     if(APPLE)
       file( GLOB ParaView_LIBRARIES "${ParaView_DIR}/*.dylib")
     elseif(UNIX)
       file( GLOB ParaView_LIBRARIES "${ParaView_DIR}/*.so")
     endif()
   endif()
  endif()
else()
  coolfluid_log_file("Qt was not found, skipping ParaView.")
endif()

coolfluid_log_file("ParaView_FOUND: [${ParaView_FOUND}]")

if( ParaView_FOUND )
  coolfluid_log_file("ParaView version: [${PARAVIEW_VERSION_MAJOR}.${PARAVIEW_VERSION_MINOR}.${PARAVIEW_VERSION_PATCH}]")
  coolfluid_log_file("ParaView library dir: ${PARAVIEW_LIBRARY_DIRS}" )
  coolfluid_log_file("ParaView libraries: ${ParaView_LIBRARIES}" )

  # add paraview libraries to list of third party libraries
  list( APPEND CF3_DEPS_LIBRARIES ${ParaView_LIBRARIES} )
endif()

coolfluid_set_package( PACKAGE ParaView
                       DESCRIPTION "Parallel visualization"
                       URL "http://www.paraview.org"
                       TYPE OPTIONAL
                       VARS ParaView_FOUND PARAVIEW_LIBRARY_DIRS ParaView_LIBRARIES )

