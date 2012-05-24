#########################################################################################
# Generic options
#########################################################################################

# code compilation options
option( CF3_ENABLE_GUI                "Enable GUI building"                             ON  )

option( CF3_ENABLE_PYTHON             "Enable building of the python bindings"          ON  )

option( CF3_ENABLE_PROTO              "Build boost::proto related code for expressions" ON  )
coolfluid_set_feature( Proto ${CF3_ENABLE_PROTO} "boost proto expressions" )

option( CF3_ENABLE_DOCS               "Enable build of documentation"                  ON  )
option( CF3_ENABLE_EXPLICIT_TEMPLATES "Enable explicit template instantiation"         ON  )
option( CF3_ENABLE_WARNINGS           "Enable lots of warnings while compiling"        OFF )
option( CF3_ENABLE_STDASSERT          "Enable standard assert() functions "            OFF )
option( CF3_ENABLE_SANDBOX            "Enable build of sandbox projects"               OFF )

option( CF3_ENABLE_VECTORIZATION      "Enable floating point vectorization"            ON  )

option( CF3_ENABLE_GPU                "Enable GPU computing    (if available)"         OFF )

option( CF3_ENABLE_CUDA               "Enable CUDA for GPGPU   (if available)"         ON  )
option( CF3_ENABLE_OPENCL             "Enable OpenCL for GPGPU (if available)"         ON  )

option( CF3_ENABLE_TCMALLOC           "Google tcmalloc (can be faster, but buggy)"     OFF )

option( CF3_ENABLE_STDDEBUG           "Enable debug of STL code"                       OFF )

set( CF3_EXTRA_DEFINES "" CACHE STRING "Extra defines or undefines to pass (examples: -DNDEBUG or -UNDEBUG)" )

# precision for real numbers

set( CF3_USER_PRECISION "DOUBLE" CACHE STRING "Precision for floating point numbers" )

# code analysis options

option( CF3_ENABLE_CODECOVERAGE       "Enable code coverage"           OFF ) # note that it turns off optimization
option( CF3_ENABLE_PROFILING          "Enable code profiling"          OFF )

option( CF3_CHECK_ORPHAN_FILES        "Check for files in the source tree that are not used" ON )
option( CF3_ENABLE_COMPONENT_TIMING   "Enables global timing of action execution. Should be turned off for final production builds" OFF )

# testing options

option( CF3_ENABLE_UNIT_TESTS        "Enable creation of unit tests"    ON  )
option( CF3_ENABLE_PERFORMANCE_TESTS "Run the performance tests"        OFF )
option( CF3_ENABLE_ACCEPTANCE_TESTS  "Run the acceptance tests"         ON  )

option( CF3_INSTALL_UNIT_TESTS       "Enable testing applications install"   OFF )

# MPI testing options

option( CF3_MPI_TESTS_RUN             "Run the MPI tests"               ON  )
option( CF3_ALL_UTESTS_PARALLEL       "Run the all the utests with mpi" OFF )
option( CF3_MPI_TESTS_RUN_SCALABILITY "Run the MPI scalability tests"   OFF )

set( CF3_MPI_TESTS_NB_PROCS     "1" CACHE STRING "Number of processes for the regular MPI tests")
set( CF3_MPI_TESTS_MAX_NB_PROCS "4" CACHE STRING "Maximum number of processes for the MPI scalability tests")
set( CF3_MPI_TESTS_SIZE         "4" CACHE STRING "Size description of the MPI tests. Interpretation is test-dependent, but higher numbers mean more RAM. 4 should be safe on a regular PC")

mark_as_advanced(CF3_MPI_TESTS_NB_PROCS)
mark_as_advanced(CF3_MPI_TESTS_MAX_NB_PROCS)
mark_as_advanced(CF3_MPI_TESTS_SIZE)
