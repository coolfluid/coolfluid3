##############################################################################
# package instrucitons
# if CPack is available and this is a build of just COOLFluiD
# as opposed to a build of from an external project
##############################################################################

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  if( "${coolfluid_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}" )

      set(CPACK_GENERATOR TGZ)
      set(CPACK_PACKAGE_NAME "COOLFluiD")
      set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "COOLFluiD - Collaborative Simulation Environment")
      set(CPACK_PACKAGE_VENDOR "von Karman Institute for Fluid Dynamics")
#       set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
#       set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
      set(CPACK_PACKAGE_VERSION "${CF_KERNEL_VERSION}")
      set(CPACK_PACKAGE_VERSION_MAJOR ${CF_KERNEL_VERSION_MAJOR})
      set(CPACK_PACKAGE_VERSION_MINOR ${CF_KERNEL_VERSION_MINOR})
      set(CPACK_PACKAGE_VERSION_PATCH ${CF_KERNEL_VERSION_PATCH})
      set(CPACK_PACKAGE_INSTALL_DIRECTORY "coolfluid-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
      set(CPACK_SOURCE_PACKAGE_FILE_NAME  "coolfluid-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
 
      # later add each component into this list
      set(CPACK_COMPONENTS_ALL applications libraries headers)
      set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Core Applications")
      set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Kernel Libraries")
      set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Kernel C++ Headers")

      include(CPack)

  endif()
endif()
