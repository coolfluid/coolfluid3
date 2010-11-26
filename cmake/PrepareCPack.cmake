##############################################################################
# package instrucitons
# if CPack is available and this is a build of just COOLFluiD
# as opposed to a build of from an external project
##############################################################################

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  if( "${coolfluid_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}" )

      #set(CPACK_GENERATOR TGZ)
      set(CPACK_PACKAGE_NAME "COOLFluiD")
      set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "COOLFluiD - Collaborative Simulation Environment")
      set(CPACK_PACKAGE_VENDOR "von Karman Institute for Fluid Dynamics")
#       set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
      set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/doc/lgpl.txt")
      set(CPACK_PACKAGE_VERSION "${CF_KERNEL_VERSION}")
      set(CPACK_PACKAGE_VERSION_MAJOR ${CF_KERNEL_VERSION_MAJOR})
      set(CPACK_PACKAGE_VERSION_MINOR ${CF_KERNEL_VERSION_MINOR})
      set(CPACK_PACKAGE_VERSION_PATCH ${CF_KERNEL_VERSION_PATCH})
      set(CPACK_PACKAGE_INSTALL_DIRECTORY "coolfluid-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
      set(CPACK_PACKAGE_RELOCATABLE "true")


      set(CPACK_SOURCE_GENERATOR TGZ)
      set(CPACK_SOURCE_PACKAGE_FILE_NAME  "coolfluid-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}-source")

      # later add each component into this list
      set(CPACK_COMPONENTS_ALL applications libraries headers)
      set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Core Applications")
      set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Kernel Libraries")
      set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Kernel C++ Headers")

      # FIXME : CPack considers Boost and MPI as system dependencies and does
      # not copy the files into the installer, so we copy them manually
      install(FILES ${Boost_LIBRARIES} ${MPI_LIBRARIES}
              DESTINATION ${CF_INSTALL_LIB_DIR}
              COMPONENT libraries)

      # platform specific configuration
      #(shamefully copied from gsmh build system for the Apple part)
      if(APPLE)
        set(CPACK_GENERATOR PackageMaker)
        set(CPACK_BUNDLE_NAME "coolfluid")
        file(READ ${CMAKE_SOURCE_DIR}/doc/coolfluid-macos.plist F0)
        string(REPLACE CF_VERSION "${CF_KERNEL_VERSION}" F1 "${F0}")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Info.plist "${F1}")
        set(CPACK_BUNDLE_PLIST ${CMAKE_BINARY_DIR}/Info.plist)
      elseif(UNIX)
         # RPM & DEB config
      elseif(WIN32)
        set(CPACK_GENERATOR NSIS)
        # Windows config
      endif()

      # CPack has to be included after variables configuration
      include(CPack)

  endif()
endif()
