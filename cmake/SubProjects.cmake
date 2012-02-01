# CTEST_BINARY_DIRECTORY must be defined in the ctest driver script before including this file
set(PROJECT_XML ${CTEST_BINARY_DIRECTORY}/Project.xml)

# Definition of the kernel subproject
set(CF3_KERNEL_PROJECT "Kernel")
set(CF3_KERNEL_LIB_DIR ${CTEST_BINARY_DIRECTORY}/cf3)
set(CF3_KERNEL_TEST_DIR ${CTEST_BINARY_DIRECTORY}/test)

# List of existing subprojects. Each subproject corresponds to a plugin. Note that this list should be kept up-to-date with the plugins that exist
# in the current source tree. It is independent of what gets actually built, since removing an item from this list
# will remove it from the dashboard. The names here must be the same as the plugin subdirectory (case sensitive!)
list(APPEND CF3_SUBPROJECTS
  "BlockMeshReader"
  "CGAL"
  "Physics"
  "RDM"
  "RiemannSolvers"
  "UFEM"
  "sdm"
  "ui"
)

# The folder tat contains the plugins build tree
set(CF3_SUBPROJECT_BASE_DIR ${CTEST_BINARY_DIRECTORY}/plugins)

# generate the Project.xml file
file(WRITE  ${PROJECT_XML} "<Project name=\"coolfluid\">\n")
file(APPEND ${PROJECT_XML} "  <SubProject name=\"${CF3_KERNEL_PROJECT}\">\n")
file(APPEND ${PROJECT_XML} "  </SubProject>\n")
foreach( subproj ${CF3_SUBPROJECTS} )
  file(APPEND ${PROJECT_XML} "  <SubProject name=\"${subproj}\">\n")
  file(APPEND ${PROJECT_XML} "  </SubProject>\n")
endforeach()
file(APPEND ${PROJECT_XML} "</Project>\n")