#########################################################################################
# Generic options
#########################################################################################

# user option to add assertions
option ( CF_ENABLE_ASSERTIONS "Enable code assertions" ON )

# precision real numbers
set ( CF_USER_PRECISION "DOUBLE" CACHE STRING "Precision for floating point numbers" )

# user option to add tracing
option ( CF_ENABLE_TRACE              "Enable tracing code"                     ON )
option ( CF_ENABLE_LOGALL 	          "Enable logging via CFLog facility"       ON )
option ( CF_ENABLE_LOGDEBUG           "Enable debug logging via CFLog facility" ON )
option ( CF_ENABLE_DEBUG_MACROS       "Enable debug macros"                     ON )

option ( CF_ENABLE_MPI                "Enable MPI compilation"                  ON   )
option ( CF_ENABLE_DOCS               "Enable build of documentation"           ON   )
option ( CF_ENABLE_EXPLICIT_TEMPLATES "Enable explicit template instantiation"  ON   )
option ( CF_ENABLE_GROWARRAY          "Enable GrowArray usage"                  ON   )
option ( CF_ENABLE_INTERNAL_DEPS      "Enable internal dependencies between libraries" ON  )
option ( CF_ENABLE_TESTCASES          "Enable checking testcases from CMake system"    OFF )
option ( CF_ENABLE_UNITTESTS          "Enable creation of unit tests"                  ON  )
option ( CF_ENABLE_WARNINGS           "Enable lots of warnings while compiling"        ON  )
option ( CF_ENABLE_STDASSERT          "Enable standard assert() functions "            OFF )
option ( CF_ENABLE_SANDBOX            "Enable build of sandbox projects"               OFF   )

# user option to add system depedent profiling
option ( CF_ENABLE_PROFILING    "Enable code profiling"                 OFF )

# user option to static build
option ( CF_ENABLE_STATIC       "Enable static building"                OFF)

option ( CF_INSTALL_TESTS       "Enable testing applications install"   OFF)


