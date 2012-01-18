#######################################################################################

  exec_program(${CMAKE_CXX_COMPILER}
               ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
               OUTPUT_VARIABLE CF3_CXX_COMPILER_VERSION)

  string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1.\\2"
         CF3_CXX_COMPILER_VERSION ${CF3_CXX_COMPILER_VERSION})

  coolfluid_log_file( "+++++  Checking C++ compiler version  -- ${CMAKE_CXX_COMPILER_ID} ${CF3_CXX_COMPILER_VERSION}" )

#######################################################################################

  coolfluid_log_file( "+++++  Checking sizes of types" )
  CHECK_TYPE_SIZE("void *"       CF3_SIZEOF_PTR)
  CHECK_TYPE_SIZE(char           CF3_SIZEOF_CHAR)
  CHECK_TYPE_SIZE(short          CF3_SIZEOF_SHORT)
  CHECK_TYPE_SIZE(int            CF3_SIZEOF_INT)
  CHECK_TYPE_SIZE(long           CF3_SIZEOF_LONG)
  CHECK_TYPE_SIZE(float          CF3_SIZEOF_FLOAT)
  CHECK_TYPE_SIZE(double         CF3_SIZEOF_DOUBLE)
  CHECK_TYPE_SIZE("long double"  CF3_SIZEOF_LONG_DOUBLE)
#   CHECK_TYPE_SIZE(int8_t         CF3_SIZEOF_INT8_T)
#   CHECK_TYPE_SIZE(uint8_t        CF3_SIZEOF_UINT8_T)
#   CHECK_TYPE_SIZE(int_least8_t   CF3_SIZEOF_INT_LEAST8_T)
#   CHECK_TYPE_SIZE(uint_least8_t  CF3_SIZEOF_UINT_LEAST8_T)
#   CHECK_TYPE_SIZE(int_fast8_t    CF3_SIZEOF_INT_FAST8_T)
#   CHECK_TYPE_SIZE(uint_fast8_t   CF3_SIZEOF_UINT_FAST8_T)
#   CHECK_TYPE_SIZE(int16_t        CF3_SIZEOF_INT16_T)
#   CHECK_TYPE_SIZE(uint16_t       CF3_SIZEOF_UINT16_T)
#   CHECK_TYPE_SIZE(int_least16_t  CF3_SIZEOF_INT_LEAST16_T)
#   CHECK_TYPE_SIZE(uint_least16_t CF3_SIZEOF_UINT_LEAST16_T)
#   CHECK_TYPE_SIZE(int_fast16_t   CF3_SIZEOF_INT_FAST16_T)
#   CHECK_TYPE_SIZE(uint_fast16_t  CF3_SIZEOF_UINT_FAST16_T)
#   CHECK_TYPE_SIZE(int32_t        CF3_SIZEOF_INT32_T)
#   CHECK_TYPE_SIZE(uint32_t       CF3_SIZEOF_UINT32_T)
#   CHECK_TYPE_SIZE(int_least32_t  CF3_SIZEOF_INT_LEAST32_T)
#   CHECK_TYPE_SIZE(uint_least32_t CF3_SIZEOF_UINT_LEAST32_T)
#   CHECK_TYPE_SIZE(int_fast32_t   CF3_SIZEOF_INT_FAST32_T)
#   CHECK_TYPE_SIZE(uint_fast32_t  CF3_SIZEOF_UINT_FAST32_T)
#   CHECK_TYPE_SIZE(int64_t        CF3_SIZEOF_INT64_T)
#   CHECK_TYPE_SIZE(uint64_t       CF3_SIZEOF_UINT64_T)
#   CHECK_TYPE_SIZE(int_least64_t  CF3_SIZEOF_INT_LEAST64_T)
#   CHECK_TYPE_SIZE(uint_least64_t CF3_SIZEOF_UINT_LEAST64_T)
#   CHECK_TYPE_SIZE(int_fast64_t   CF3_SIZEOF_INT_FAST64_T)
#   CHECK_TYPE_SIZE(uint_fast64_t  CF3_SIZEOF_UINT_FAST64_T)
  CHECK_TYPE_SIZE(size_t         CF3_SIZEOF_SIZE_T)
#   CHECK_TYPE_SIZE(ssize_t        CF3_SIZEOF_SSIZE_T)
#   CHECK_TYPE_SIZE(off_t          CF3_SIZEOF_OFF_T)
#   CHECK_TYPE_SIZE(__int64        CF3_SIZEOF___INT64)
  CHECK_TYPE_SIZE("long long"    CF3_SIZEOF_LONG_LONG)

  math( EXPR CF3_OS_BITS "${CF3_SIZEOF_PTR} * 8")

#######################################################################################

  # coolfluid_log( "+++++  Checking for pre compiled header support" )
  # include(CheckPreCompiledHeaderSupport)

#######################################################################################

include(CheckNullPtr)

#######################################################################################

  check_cxx_source_compiles(
  " namespace lolo { struct popo { int i; };  }
    using namespace lolo;
    int main(int argc, char* argv[])
    {
      lolo::popo p;
      popo pp;
      return 0;
    }
  "
  CF3_CXX_HAVE_NAMESPACES)
  if( NOT CF3_CXX_HAVE_NAMESPACES )
    message( FATAL_ERROR "C++ compiler does not support namespaces" )
  endif()
  coolfluid_log_file( "+++++  Checking C++ compiler has namespaces -- ${CF3_CXX_HAVE_NAMESPACES}" )

#######################################################################################

  check_cxx_source_compiles(
  "#include <iostream>
    int main(int argc, char* argv[])
    {
      std::cout << __FUNCTION__ << std::endl;
    }"
  CF3_HAVE_FUNCTION_DEF )
  coolfluid_log_file( "+++++  Checking for __FUNCTION__ support -- ${CF3_HAVE_FUNCTION_DEF}" )

#######################################################################################

  include(CheckRestrictKeyword)

#######################################################################################

  include( CheckExplicitInstantiation )

#######################################################################################

  # check memory mmap functions
  check_function_exists(mmap   CF3_HAVE_MMAP)
  check_function_exists(munmap CF3_HAVE_MUNMAP)
  check_function_exists(mremap CF3_HAVE_MREMAP)
  if(CF3_HAVE_MMAP AND CF3_HAVE_MUNMAP AND CF3_HAVE_MREMAP)
    set( CF3_HAVE_ALLOC_MMAP 1 CACHE BOOL "MemoryAllocator_MMAP can be built")
  endif()

  coolfluid_log_file( "+++++  Checking for mmap support -- ${CF3_HAVE_ALLOC_MMAP}" )

#######################################################################################

  check_function_exists(vsnprintf   CF3_HAVE_VSNPRINTF)

  coolfluid_log_file( "+++++  Checking for vsnprintf function -- ${CF3_HAVE_VSNPRINTF}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { erfc (0.); }
  "
  CF3_HAVE_MATH_ERFC )

  coolfluid_log_file( "+++++  Checking for erfc function -- ${CF3_HAVE_MATH_ERFC}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::erfc (0.); }
  "
  CF3_HAVE_MATH_TR1_ERFC )

  coolfluid_log_file( "+++++  Checking for std::tr1::erfc function -- ${CF3_HAVE_MATH_TR1_ERFC}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { cbrt (0.); }
  "
  CF3_HAVE_MATH_CBRT )

  coolfluid_log_file( "+++++  Checking for cbrt function -- ${CF3_HAVE_MATH_CBRT}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::cbrt (0.); }
  "
  CF3_HAVE_MATH_TR1_CBRT )

  coolfluid_log_file( "+++++  Checking for std::tr1::cbrt function -- ${CF3_HAVE_MATH_TR1_CBRT}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { log2 (0.); }
  "
  CF3_HAVE_MATH_LOG2 )

  coolfluid_log_file( "+++++  Checking for log2 function -- ${CF3_HAVE_MATH_LOG2}" )

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { std::tr1::log2 (0.); }
  "
  CF3_HAVE_MATH_TR1_LOG2 )

  coolfluid_log_file( "+++++  Checking for std::tr1::log2 function -- ${CF3_HAVE_MATH_TR1_LOG2}" )


#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { hypot (0., 1.2); }
  "
  CF3_HAVE_MATH_HYPOT )

  coolfluid_log_file( "+++++  Checking for hypot function -- ${CF3_HAVE_MATH_HYPOT}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::hypot (0., 1.2); }
  "
  CF3_HAVE_MATH_TR1_HYPOT )

  coolfluid_log_file( "+++++  Checking for std::tr1::hypot function -- ${CF3_HAVE_MATH_TR1_HYPOT}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { exp2 (0.); }
  "
  CF3_HAVE_MATH_EXP2 )

  coolfluid_log_file( "+++++  Checking for exp2 function -- ${CF3_HAVE_MATH_EXP2}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::exp2 (0.); }
  "
  CF3_HAVE_MATH_TR1_EXP2 )

  coolfluid_log_file( "+++++  Checking for std::tr1::exp2 function -- ${CF3_HAVE_MATH_TR1_EXP2}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { asinh (0.); }
  "
  CF3_HAVE_MATH_ASINH )

  coolfluid_log_file( "+++++  Checking for asinh function -- ${CF3_HAVE_MATH_ASINH}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::asinh (0.); }
  "
  CF3_HAVE_MATH_TR1_ASINH )

  coolfluid_log_file( "+++++  Checking for std::tr1::asinh function -- ${CF3_HAVE_MATH_TR1_ASINH}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { acosh (0.); }
  "
  CF3_HAVE_MATH_ACOSH )

  coolfluid_log_file( "+++++  Checking for acosh function -- ${CF3_HAVE_MATH_ACOSH}" )

  check_cxx_source_compiles(
  " #include <tr1/cmath>
    int main (int argc, char* argv[]) { std::tr1::acosh (0.); }
  "
  CF3_HAVE_MATH_TR1_ACOSH )

  coolfluid_log_file( "+++++  Checking for std::tr1::acosh function -- ${CF3_HAVE_MATH_TR1_ACOSH}" )

#######################################################################################

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { atanh (0.); }
  "
  CF3_HAVE_MATH_ATANH)

  coolfluid_log_file( "+++++  Checking for atanh function -- ${CF3_HAVE_MATH_ATANH}" )

  check_cxx_source_compiles(
  " #include <cmath>
    int main (int argc, char* argv[]) { std::tr1::atanh (0.); }
  "
  CF3_HAVE_MATH_TR1_ATANH)

  coolfluid_log_file( "+++++  Checking for std::tr1::atanh function -- ${CF3_HAVE_MATH_TR1_ATANH}" )

#######################################################################################

  check_include_file( unistd.h  CF3_HAVE_UNISTD_H )

  coolfluid_log_file( "+++++  Checking for the POSIX unistd.h header -- ${CF3_HAVE_UNISTD_H}" )

#######################################################################################

  # check for c++ abi, usually present in GNU compilers

  check_cxx_source_compiles(
    "#include <cxxabi.h>
    int main(int argc, char* argv[])
    { char * type; int status;
      char * r = abi::__cxa_demangle(type, 0, 0, &status);
      return 0;
    }"
    CF3_HAVE_CXXABI_H)

  coolfluid_log_file( "+++++  Checking for cxxabi -- ${CF3_HAVE_CXXABI_H}" )

#######################################################################################

  # check for time headers
  coolfluid_log_file( "+++++  Checking for headers with time information" )

  check_include_file(sys/time.h       CF3_HAVE_SYS_TIME_H)
  check_include_file(time.h           CF3_HAVE_TIME_H)
  check_include_file(sys/resource.h   CF3_HAVE_SYS_RESOURCE_H)

  check_function_exists(gettimeofday  CF3_HAVE_GETTIMEOFDAY)

#######################################################################################
# Win32 specific
#######################################################################################

if(WIN32)

  coolfluid_log_file( "+++++  Checking for the Win32 windows.h header" )    # check windows.hfor Windows API
  check_include_file_cxx(windows.h CF3_HAVE_WINDOWSH)
  coolfluid_log_file( "+++++  Checking for the Win32 dbghelp.h header" )    # check dbghelp.h for call stack
  check_include_file_cxx(dbghelp.h CF3_HAVE_DBGHELPH)
  coolfluid_log_file( "+++++  Checking for the Win32 psapi.h header" )      # check psapi.h for memory info
  check_include_file_cxx(psapi.h CF3_HAVE_PSAPIH)

endif()

#######################################################################################
# UNIX specific
#######################################################################################

if(UNIX)

  # for dlopen
  check_include_file_cxx(dlfcn.h CF3_HAVE_DLOPEN)

  coolfluid_log_file( "+++++  Checking for the dlfcn.h header -- ${CF3_HAVE_DLOPEN}")

endif()

