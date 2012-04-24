set( CF3_LIBRARY_LINK_FLAGS "" CACHE STRING "Extra link flags for libraries" FORCE )
mark_as_advanced( CF3_LIBRARY_LINK_FLAGS )

########################################################################################
# GCC Compiler
########################################################################################

# gnu specific warning flags
if( CMAKE_COMPILER_IS_GNUCC )

    # use pipe for faster compilation
    coolfluid_add_c_flags("-pipe")
    coolfluid_add_cxx_flags("-pipe")

    # respect c 89 standard (same as -std=c89)
    coolfluid_add_c_flags("-ansi")
    # respect c++ 98 standard
    coolfluid_add_cxx_flags("-std=c++98")
    # dont allow gnu extensions
    coolfluid_add_cxx_flags("-fno-gnu-keywords")

    # dont define common variables
    coolfluid_add_c_flags("-fno-common")
    coolfluid_add_cxx_flags("-fno-common")


    if( CF3_ENABLE_WARNINGS )
      # use many warnings
      coolfluid_add_cxx_flags("-Wall")
      coolfluid_add_cxx_flags("-W")
      coolfluid_add_cxx_flags("-Wextra")
      coolfluid_add_cxx_flags("-Woverloaded-virtual")
      coolfluid_add_cxx_flags("-Wsign-promo")
      coolfluid_add_cxx_flags("-Wformat")
      # Warn if an undefined identifier is evaluated in an #if directive.
      coolfluid_add_cxx_flags("-Wundef" )
      #  Warn about anything that depends on the "size of" a function type or of "void"
      coolfluid_add_cxx_flags("-Wpointer-arith")
      #  warn about uses of format functions that represent possible security problems
      coolfluid_add_cxx_flags("-Wformat-security")

      # accept functions that dont use all parameters, due to virtual functions may not need all
      coolfluid_add_cxx_flags("-Wno-unused-parameter")
      coolfluid_add_cxx_flags("-Wno-missing-field-initializers")

      # Don't warn when using functors in boost::bind
      coolfluid_add_cxx_flags("-Wno-strict-aliasing")

      # this was temporary until we all move to using openmpi
      # must turn off non-virtual-dtor because many mpi implementations use it
      # KDE uses -Wnon-virtual-dtor
      # coolfluid_add_cxx_flags("-Wno-non-virtual-dtor")

      # must turn long long off because many mpi implementations use it
      coolfluid_add_cxx_flags("-Wno-long-long")
      # be pedantic but issue warnings instead of errors
      # coolfluid_add_cxx_flags("-pedantic") # Disabled for now, see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=33305
      coolfluid_add_cxx_flags("-fpermissive")


      coolfluid_add_cxx_flags("-Wno-empty-body")    # Problem in boost
      coolfluid_add_cxx_flags("-Wno-uninitialized") # Problem with boost accumulators
  endif()

endif()

########################################################################################
# INTEL ICC
########################################################################################

if( CMAKE_CXX_COMPILER_ID MATCHES "Intel" AND CF3_CXX_COMPILER_VERSION MATCHES "12")

  # suppress warning (remark) #279: controlling expression is constant
  # because of boost use of BOOST_ASSERT( expr && "message")
  coolfluid_add_cxx_flags("-wd279")

  # suppress warning #2196: routine is both "inline" and "noinline" ("noinline" assumed)
  # because of Eigen declarations such as "static EIGEN_DONT_INLINE void run()"
  coolfluid_add_cxx_flags("-wd2196")
endif()

########################################################################################
# CLANG
########################################################################################

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  coolfluid_add_cxx_flags("-Wno-parentheses -Wno-unknown-warning-option -Wno-c++11-compat")
endif()

########################################################################################
# UNIX
########################################################################################

if(UNIX)

    if( CF3_ENABLE_CODECOVERAGE )

      find_program(CTEST_COVERAGE_COMMAND gcov)

      if( CTEST_COVERAGE_COMMAND )
        set( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage" )
        set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage" )
        set( LINK_FLAGS        "${LINK_FLAGS} -fprofile-arcs -ftest-coverage" )
      endif()
    endif()

endif(UNIX)

########################################################################################
# WINDOWS
########################################################################################

if(WIN32)

  # stupid VS2005 warning about not using fopen
  add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
  # for M_PI in cmath
  add_definitions( -D_USE_MATH_DEFINES )
  # disable auto-linking with boost
  add_definitions( -DBOOST_ALL_NO_LIB )
  add_definitions( -DBOOST_ALL_DYN_LINK )

  # Required for auto link not to mess up on vs80.
  # @todo Disable auto link on windows altogether.
  # add_definitions( -DBOOST_DYN_LINK )

  # compilation flags
  #   /MD use the Multithreaded DLL of runtime library
  coolfluid_add_c_flags( "/MD" )
  coolfluid_add_cxx_flags( "/MD" )

  # add exception handling
  coolfluid_add_c_flags( "/EHsc" )
  coolfluid_add_cxx_flags( "/EHsc" )

  # remove warnings
  if( CF3_ENABLE_WARNINGS )
    coolfluid_add_c_flags( "/W3" )
    coolfluid_add_cxx_flags( "/W3" )
  else()
    coolfluid_add_c_flags( "/W0" )
    coolfluid_add_cxx_flags( "/W0" )
  endif()

  # linker flags:
  #   /OPT:NOREF keeps functions and data that are never referenced ( needed for static libs )
  set( CF3_LIBRARY_LINK_FLAGS "/OPT:NOREF /OPT:NOICF"  CACHE STRING "Extra link flags for libraries" FORCE )

  #   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOREF /OPT:NOICF" )
  #   set( CMAKE_CXX_CREATE_STATIC_LIBRARY  "lib ${CMAKE_CL_NOLOGO} /OPT:NOREF /OPT:NOICF <LINK_FLAGS> /out:<TARGET> <OBJECTS>" )

endif(WIN32)

########################################################################################
# APPLE
########################################################################################

if( APPLE )

	# improve the linker compiler to avoid unresolved symbols causing errors
  # not needed anymore because all lib depencies are explicitly set
  #  set(CMAKE_CXX_CREATE_SHARED_LIBRARY
  #  "<CMAKE_CXX_COMPILER> -undefined dynamic_lookup <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")

  # under Mac OS X internal deps must be used so force them
  if( NOT CF3_ENABLE_INTERNAL_DEPS )
    set( CF3_ENABLE_INTERNAL_DEPS ON CACHE BOOL "Use of internal deps is forced" FORCE )
  endif()

    if( CF3_ENABLE_WARNINGS )
    endif()

endif(APPLE)

########################################################################################
# FINAL
########################################################################################

# test and add the user defined flags

string( REGEX MATCHALL "[^ ]+" C_FLAGS_LIST "${CF3_C_FLAGS}"  )
foreach( c_flag ${C_FLAGS_LIST} )
  coolfluid_add_c_flags_or_die ( ${c_flag} )
endforeach()
mark_as_advanced( C_FLAGS_LIST   )

string( REGEX MATCHALL "[^ ]+" CXX_FLAGS_LIST "${CF3_CXX_FLAGS}"  )
foreach( cxx_flag ${CXX_FLAGS_LIST} )
  coolfluid_add_cxx_flags_or_die ( ${cxx_flag} )
endforeach()
mark_as_advanced( CXX_FLAGS_LIST  )

if( NOT CF3_SKIP_FORTRAN )
  string( REGEX MATCHALL "[^ ]+" Fortran_FLAGS_LIST "${CF3_Fortran_FLAGS}"  )
  # fortran flags currently nont checked
  set( CMAKE_Fortran_FLAGS "${CF3_Fortran_FLAGS}" )
  # foreach( fortran_flag ${Fortran_FLAGS_LIST} )
  #   coolfluid_add_Fortran_flags_or_die ( ${fortran_flag} )
  # endforeach()
  mark_as_advanced( Fortran_FLAGS_LISTS )
endif()

