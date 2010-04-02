#########################################################################################
# Generic OPTIONS
#########################################################################################

# user option to add assertions
OPTION ( CF_ENABLE_ASSERTIONS "Enable code assertions" ON )

# precision real numbers
OPTION ( CF_PRECISION_SINGLE       "Real numbers have single precision (will choose most precise)"       OFF )
OPTION ( CF_PRECISION_DOUBLE       "Real numbers have double precision (will choose most precise)"       ON  )
OPTION ( CF_PRECISION_LONG_DOUBLE  "Real numbers have long double precision (will choose most precise)"  OFF )

mark_as_advanced ( CF_PRECISION_SINGLE CF_PRECISION_DOUBLE CF_PRECISION_LONG_DOUBLE )

# user option to add tracing
OPTION(CF_ENABLE_TRACE 	"Enable tracing code"  ON)
# user option to add logging
OPTION(CF_ENABLE_LOGALL 	"Enable logging via CFLog facility" ON)
# user option to add debug logging
OPTION(CF_ENABLE_LOGDEBUG 	"Enable debug logging via CFLog facility" ON)
# user option to enable debug macros
OPTION(CF_ENABLE_DEBUG_MACROS 	"Enable debug macros"                 ON)

OPTION ( CF_ENABLE_MPI                "Enable MPI compilation"                  ON   )
OPTION ( CF_ENABLE_DOCS               "Enable build of documentation"           ON   )
OPTION ( CF_ENABLE_EXPLICIT_TEMPLATES "Enable explicit template instantiation"  ON   )
OPTION ( CF_ENABLE_GROWARRAY          "Enable GrowArray usage"                  ON   )
OPTION ( CF_ENABLE_INTERNAL_DEPS      "Enable internal dependencies between libraries" ON  )
OPTION ( CF_ENABLE_TESTCASES          "Enable checking testcases from CMake system"    OFF )
OPTION ( CF_ENABLE_UNITTESTS          "Enable creation of unit tests"                  ON  )
OPTION ( CF_ENABLE_WARNINGS           "Enable lots of warnings while compiling"        ON  )
OPTION ( CF_ENABLE_STDASSERT          "Enable standard assert() functions "            OFF )

# user option to add system depedent profiling
OPTION ( CF_ENABLE_PROFILING    "Enable code profiling"                 OFF )

# user option to static build
OPTION ( CF_ENABLE_STATIC       "Enable static building"                OFF)

OPTION ( CF_INSTALL_TESTS       "Enable testing applications install"   OFF)


