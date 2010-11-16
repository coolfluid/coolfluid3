#########################################################################################
# Generic options
#########################################################################################

# user option to add assertions
option( CF_ENABLE_ASSERTIONS "Enable code assertions" ON )

option( CF_ENABLE_GUI               "Enable GUI building"                     ON )

option( CF_ENABLE_TRACE              "Enable tracing code"                     ON  )
option( CF_ENABLE_LOGALL  	         "Enable logging via CFLog facility"       ON  )
option( CF_ENABLE_LOGDEBUG           "Enable debug logging via CFLog facility" ON  )
option( CF_ENABLE_STDDEBUG           "Enable debug of STL code"                OFF )
option( CF_ENABLE_DEBUG_MACROS       "Enable debug macros"                     ON  )

option( CF_ENABLE_DOCS               "Enable build of documentation"           ON   )
option( CF_ENABLE_EXPLICIT_TEMPLATES "Enable explicit template instantiation"  ON   )
option( CF_ENABLE_TESTCASES          "Enable checking testcases from CMake system"    OFF )
option( CF_ENABLE_UNITTESTS          "Enable creation of unit tests"                  ON  )
option( CF_ENABLE_WARNINGS           "Enable lots of warnings while compiling"        OFF )
option( CF_ENABLE_STDASSERT          "Enable standard assert() functions "            OFF )
option( CF_ENABLE_SANDBOX            "Enable build of sandbox projects"               OFF )

option( CF_ENABLE_VECTORIZATION      "Enable floating point vectorization"            ON  )


# option to do code coverage
# note that it runs off optimization
option( CF_ENABLE_CODECOVERAGE              "Enable code coverage"     OFF )

# user option to add system depedent profiling
option( CF_ENABLE_PROFILING    "Enable code profiling"                 OFF )
option( CF_USE_TCMALLOC    "Use google perftools tcmalloc (can be faster, but buggy)"                 OFF )

# user option to static build
option( CF_ENABLE_STATIC       "Enable static building"                OFF)

option( CF_INSTALL_TESTS       "Enable testing applications install"   OFF)

# precision real numbers
set( CF_USER_PRECISION "DOUBLE" CACHE STRING "Precision for floating point numbers" )

option( CF_CHECK_ORPHAN_FILES  "If turned on, build system checks for files in the source tree that are not used in CMakeLists.txt files" ON)
mark_as_advanced( CF_CHECK_ORPHAN_FILES )

# MPI testing options
option( CF_MPI_TESTS_RUN "Run the MPI tests" OFF)
option( CF_MPI_TESTS_RUN_SCALABILITY "Run the MPI scalability tests" OFF)
set( CF_MPI_TESTS_NB_PROCS "4" CACHE STRING "Number of processes for the regular MPI tests")
set( CF_MPI_TESTS_MAX_NB_PROCS "4" CACHE STRING "Maximum number of processes for the MPI scalability tests")
set(CF_MPI_TESTS_SIZE "4" CACHE STRING "Size description of the MPI tests. Interpretation is test-dependent, but higher numbers mean more RAM. 4 should be safe on a regular PC")
mark_as_advanced(CF_MPI_TESTS_RUN)
mark_as_advanced(CF_MPI_TESTS_RUN_SCALABILITY)
mark_as_advanced(CF_MPI_TESTS_NB_PROCS)
mark_as_advanced(CF_MPI_TESTS_MAX_NB_PROCS)
mark_as_advanced(CF_MPI_TESTS_SIZE)
