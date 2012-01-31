##########################################################################
# Dashboard configuration
##########################################################################

# dashboard model
set(MODEL Experimental)

# source dir
set(CTEST_SOURCE_DIRECTORY "/home/bjanssens/src/coolfluid/coolfluid3-workspace/coolfluid")
# build dir
set(CTEST_BINARY_DIRECTORY "/home/bjanssens/src/build/coolfluid3-clang-release")
# dependencies
set(CF_DEPS_ROOT "/home/bjanssens/src/build/coolfluid-deps-clang")
# build type
set(CF_BUILD_TYPE "Release")
# platform
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
# site config
set(CTEST_SITE "BartHome")
set(CTEST_BUILD_NAME "clang-Release-home")
# parallel build config
set(CTEST_BUILD_FLAGS "-j6")
# commands
set(CTEST_UPDATE_COMMAND "git")
# cmake configuration commmand
set(CTEST_CMAKE_COMMAND "cmake")

##########################################################################
# Build configuration
##########################################################################

set(CF_CONFIG_OPTIONS "-DDEPS_ROOT=${CF_DEPS_ROOT} \"-DCTEST_BUILD_FLAGS:STRING=${CTEST_BUILD_FLAGS}\" -DCMAKE_BUILD_TYPE:STRING=${CF_BUILD_TYPE} ")
set(CF_CONFIG_OPTIONS "${CF_CONFIG_OPTIONS} -DSITE:STRING=${CTEST_SITE} -DBUILDNAME:STRING=${CTEST_BUILD_NAME}")
set(CF_CONFIG_OPTIONS "${CF_CONFIG_OPTIONS} -DCF_ENABLE_ACCEPTANCE_TESTS=ON -DCF_ENABLE_SANDBOX=ON -DCTEST_USE_LAUNCHERS=1")

set(CTEST_CONFIGURE_COMMAND "${CTEST_CMAKE_COMMAND} ${CF_CONFIG_OPTIONS} \"-G${CTEST_CMAKE_GENERATOR}\" \"${CTEST_SOURCE_DIRECTORY}\"")
#for running inside existing build dir
# set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"${CTEST_BINARY_DIRECTORY}\"")

##########################################################################
# Dashboard execution
##########################################################################

include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/cmake/SubProjects.cmake")

ctest_submit(FILES "${CTEST_BINARY_DIRECTORY}/Project.xml")

set(START_TIME ${CTEST_ELAPSED_TIME})
ctest_start (${MODEL})
# ctest_update(RETURN_VALUE HAD_UPDATES)
ctest_submit(PARTS Update Notes)

set_property(GLOBAL PROPERTY SubProject ${CF3_KERNEL_PROJECT})
set_property(GLOBAL PROPERTY Label ${CF3_KERNEL_PROJECT})

ctest_configure(OPTIONS "")
ctest_submit(PARTS Configure)

# Test the kernel libs
ctest_build(BUILD ${CF3_KERNEL_LIB_DIR} APPEND)
ctest_build(BUILD ${CF3_KERNEL_TEST_DIR} APPEND)
ctest_submit(PARTS Build)
ctest_test(BUILD ${CF3_KERNEL_TEST_DIR})
ctest_submit(PARTS Test)

foreach(subproject ${CF3_SUBPROJECTS})
  set_property(GLOBAL PROPERTY SubProject ${subproject})
  set_property(GLOBAL PROPERTY Label ${subproject})
  ctest_build(BUILD ${CF3_SUBPROJECT_BASE_DIR}/${subproject} APPEND) # builds target ${CTEST_BUILD_TARGET}
  ctest_submit(PARTS Build)
  ctest_test(BUILD ${CF3_SUBPROJECT_BASE_DIR}/${subproject})
  ctest_submit(PARTS Test)
endforeach()
