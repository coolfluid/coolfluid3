## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.

set(CTEST_PROJECT_NAME       "coolfluid")
set(CTEST_NIGHTLY_START_TIME "23:00:00 GMT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "coolfluidsrv.vki.ac.be")
set(CTEST_DROP_LOCATION "/cdash/submit.php?project=coolfluid")
set(CTEST_DROP_SITE_CDASH TRUE)

set(CTEST_UPDATE_TYPE "git")

