#########################################################################################
# Generic options
#########################################################################################

# code compilation options

option( CF_ENABLE_ASSERTIONS         "Enable code assertions" ON )
option( CF_ENABLE_GUI                "Enable GUI building"                     ON )

option( CF_ENABLE_DOCS               "Enable build of documentation"           ON   )
option( CF_ENABLE_EXPLICIT_TEMPLATES "Enable explicit template instantiation"  ON   )
option( CF_ENABLE_WARNINGS           "Enable lots of warnings while compiling"        OFF )
option( CF_ENABLE_STDASSERT          "Enable standard assert() functions "            OFF )
option( CF_ENABLE_SANDBOX            "Enable build of sandbox projects"               OFF )

option( CF_ENABLE_VECTORIZATION      "Enable floating point vectorization"            ON  )

option( CF_ENABLE_GPU                "Enable GPU computing  (if available)"           OFF )
# add_feature_info(GPU CF_ENABLE_GPU   "GPU provides accelerated computations using graphics processing units")
option( CF_ENABLE_CUDA               "Enable CUDA for GPGPU (if available)"           ON  )
option( CF_ENABLE_OPENCL             "Enable OpenCL for GPGPU (if available)"         ON  )

option( CF_ENABLE_TCMALLOC           "Use google perftools tcmalloc (can be faster, but buggy)"  OFF )

option( CF_ENABLE_STATIC             "Enable static building"         OFF )

# precision for real numbers

set( CF_USER_PRECISION "DOUBLE" CACHE STRING "Precision for floating point numbers" )

# log options

option( CF_ENABLE_TRACE              "Enable tracing code"                     ON  )
option( CF_ENABLE_LOGALL  	         "Enable logging via CFLog facility"       ON  )
option( CF_ENABLE_LOGDEBUG           "Enable debug logging via CFLog facility" ON  )
option( CF_ENABLE_STDDEBUG           "Enable debug of STL code"                OFF )
option( CF_ENABLE_DEBUG_MACROS       "Enable debug macros"                     ON  )

# code analysis options

option( CF_ENABLE_CODECOVERAGE       "Enable code coverage"           OFF ) # note that it turns off optimization
option( CF_ENABLE_PROFILING          "Enable code profiling"          OFF )

option( CF_CHECK_ORPHAN_FILES        "Check for files in the source tree that are not used" ON )

# testing options

option( CF_ENABLE_UNIT_TESTS        "Enable creation of unit tests"    ON  )
option( CF_ENABLE_PERFORMANCE_TESTS "Run the performance tests"        OFF )
option( CF_ENABLE_ACCEPTANCE_TESTS  "Run the acceptance tests"         OFF )

option( CF_INSTALL_UNIT_TESTS       "Enable testing applications install"   OFF )

# MPI testing options

option( CF_MPI_TESTS_RUN             "Run the MPI tests"               ON )
option( CF_MPI_TESTS_RUN_SCALABILITY "Run the MPI scalability tests"   OFF )

set( CF_MPI_TESTS_NB_PROCS     "4" CACHE STRING "Number of processes for the regular MPI tests")
set( CF_MPI_TESTS_MAX_NB_PROCS "4" CACHE STRING "Maximum number of processes for the MPI scalability tests")
set( CF_MPI_TESTS_SIZE         "4" CACHE STRING "Size description of the MPI tests. Interpretation is test-dependent, but higher numbers mean more RAM. 4 should be safe on a regular PC")

mark_as_advanced(CF_MPI_TESTS_NB_PROCS)
mark_as_advanced(CF_MPI_TESTS_MAX_NB_PROCS)
mark_as_advanced(CF_MPI_TESTS_SIZE)
