coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( "Checking compiler features:" )
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )

#######################################################################################

  coolfluid_log( "+++++  Checking sizes of types" )
  CHECK_TYPE_SIZE("void *"       CF_SIZEOF_PTR)
  CHECK_TYPE_SIZE(char           CF_SIZEOF_CHAR)
  CHECK_TYPE_SIZE(short          CF_SIZEOF_SHORT)
  CHECK_TYPE_SIZE(int            CF_SIZEOF_INT)
  CHECK_TYPE_SIZE(long           CF_SIZEOF_LONG)
  CHECK_TYPE_SIZE(float          CF_SIZEOF_FLOAT)
  CHECK_TYPE_SIZE(double         CF_SIZEOF_DOUBLE)
  CHECK_TYPE_SIZE("long double"  CF_SIZEOF_LONG_DOUBLE)
#   CHECK_TYPE_SIZE(int8_t         CF_SIZEOF_INT8_T)
#   CHECK_TYPE_SIZE(uint8_t        CF_SIZEOF_UINT8_T)
#   CHECK_TYPE_SIZE(int_least8_t   CF_SIZEOF_INT_LEAST8_T)
#   CHECK_TYPE_SIZE(uint_least8_t  CF_SIZEOF_UINT_LEAST8_T)
#   CHECK_TYPE_SIZE(int_fast8_t    CF_SIZEOF_INT_FAST8_T)
#   CHECK_TYPE_SIZE(uint_fast8_t   CF_SIZEOF_UINT_FAST8_T)
#   CHECK_TYPE_SIZE(int16_t        CF_SIZEOF_INT16_T)
#   CHECK_TYPE_SIZE(uint16_t       CF_SIZEOF_UINT16_T)
#   CHECK_TYPE_SIZE(int_least16_t  CF_SIZEOF_INT_LEAST16_T)
#   CHECK_TYPE_SIZE(uint_least16_t CF_SIZEOF_UINT_LEAST16_T)
#   CHECK_TYPE_SIZE(int_fast16_t   CF_SIZEOF_INT_FAST16_T)
#   CHECK_TYPE_SIZE(uint_fast16_t  CF_SIZEOF_UINT_FAST16_T)
#   CHECK_TYPE_SIZE(int32_t        CF_SIZEOF_INT32_T)
#   CHECK_TYPE_SIZE(uint32_t       CF_SIZEOF_UINT32_T)
#   CHECK_TYPE_SIZE(int_least32_t  CF_SIZEOF_INT_LEAST32_T)
#   CHECK_TYPE_SIZE(uint_least32_t CF_SIZEOF_UINT_LEAST32_T)
#   CHECK_TYPE_SIZE(int_fast32_t   CF_SIZEOF_INT_FAST32_T)
#   CHECK_TYPE_SIZE(uint_fast32_t  CF_SIZEOF_UINT_FAST32_T)
#   CHECK_TYPE_SIZE(int64_t        CF_SIZEOF_INT64_T)
#   CHECK_TYPE_SIZE(uint64_t       CF_SIZEOF_UINT64_T)
#   CHECK_TYPE_SIZE(int_least64_t  CF_SIZEOF_INT_LEAST64_T)
#   CHECK_TYPE_SIZE(uint_least64_t CF_SIZEOF_UINT_LEAST64_T)
#   CHECK_TYPE_SIZE(int_fast64_t   CF_SIZEOF_INT_FAST64_T)
#   CHECK_TYPE_SIZE(uint_fast64_t  CF_SIZEOF_UINT_FAST64_T)
  CHECK_TYPE_SIZE(size_t         CF_SIZEOF_SIZE_T)
#   CHECK_TYPE_SIZE(ssize_t        CF_SIZEOF_SSIZE_T)
#   CHECK_TYPE_SIZE(off_t          CF_SIZEOF_OFF_T)
#   CHECK_TYPE_SIZE(__int64        CF_SIZEOF___INT64)
  CHECK_TYPE_SIZE("long long"    CF_SIZEOF_LONG_LONG)

  math( EXPR CF_OS_BITS "${CF_SIZEOF_PTR} * 8")

#######################################################################################

  # coolfluid_log( "+++++  Checking for pre compiled header support" )
  # include(CheckPreCompiledHeaderSupport)

#######################################################################################

coolfluid_log( "+++++  Checking for pre compiled header support" )
include(CheckNullPtr)

#######################################################################################

  coolfluid_log( "+++++  Checking C++ compiler has namespaces" )
  check_cxx_source_compiles (
  " namespace lolo { struct popo { int i; };  }
    using namespace lolo;
    int main(int argc, char* argv[])
    {
      lolo::popo p;
      popo pp;
      return 0;
    }
  "
  CF_CXX_HAVE_NAMESPACES)
  if( NOT CF_CXX_HAVE_NAMESPACES )
    message( FATAL_ERROR "C++ compiler does not support namespaces" )
  endif()

#######################################################################################

  coolfluid_log( "+++++  Checking for __FUNCTION__ support" )
  include(CheckFunctionDef)

#######################################################################################

  coolfluid_log( "+++++  Checking for restrict keyword" )
  include(CheckRestrictKeyword)

#######################################################################################

  # coolfluid_log( "+++++  Checking for explicit template support" ) # output in macro
  include( CheckExplicitInstantiation )

#######################################################################################

  coolfluid_log( "+++++  Checking for mmap support" ) # check memory mmap functions
  # check memory mmap functions
  check_function_exists(mmap   CF_HAVE_MMAP)
  check_function_exists(munmap CF_HAVE_MUNMAP)
  check_function_exists(mremap CF_HAVE_MREMAP)
  if(CF_HAVE_MMAP AND CF_HAVE_MUNMAP AND CF_HAVE_MREMAP)
    set( CF_HAVE_ALLOC_MMAP 1 CACHE BOOL "MemoryAllocator_MMAP can be built")
  endif()

#######################################################################################

  coolfluid_log( "+++++  Checking for vsnprintf function" ) # check memory mmap functions
  check_function_exists(vsnprintf   CF_HAVE_VSNPRINTF)

#######################################################################################

  coolfluid_log( "+++++  Checking for erfc function" )
  check_cxx_source_compiles (
  " #include <cmath>
    int main (int argc, char* argv[]) { erfc (0.); }
  "
  CF_HAVE_MATH_ERFC )

#######################################################################################

  coolfluid_log( "+++++  Checking for asinh function" )
  check_cxx_source_compiles (
  " #include <cmath>
    int main (int argc, char* argv[]) { asinh (0.); }
  "
  CF_HAVE_MATH_ASINH )

#######################################################################################

  coolfluid_log( "+++++  Checking for acosh function" )
  check_cxx_source_compiles (
  " #include <cmath>
    int main (int argc, char* argv[]) { acosh (0.); }
  "
  CF_HAVE_MATH_ACOSH )

#######################################################################################

  coolfluid_log( "+++++  Checking for atanh function" )
  check_cxx_source_compiles (
  " #include <cmath>
    int main (int argc, char* argv[]) { atanh (0.); }
  "
  CF_HAVE_MATH_ATANH)
  
#######################################################################################

  coolfluid_log( "+++++  Checking for the POSIX unistd.h header" )    # check unistd.h
  check_include_file( unistd.h  CF_HAVE_UNISTD_H )

#######################################################################################

  # check for c++ abi, ussually present in GNU compilers
  coolfluid_log( "+++++  Checking for cxxabi" )

  check_cxx_source_compiles (
    "#include <cxxabi.h>
    int main(int argc, char* argv[])
    { char * type; int status;
      char * r = abi::__cxa_demangle(type, 0, 0, &status);
      return 0;
    }"
    CF_HAVE_CXXABI_H)

#######################################################################################

  # check for time headers
  coolfluid_log( "+++++  Checking for headers with time information" )
  check_include_file(sys/time.h       CF_HAVE_SYS_TIME_H)
  check_include_file(time.h           CF_HAVE_TIME_H)
  check_include_file(sys/resource.h   CF_HAVE_SYS_RESOURCE_H)
  check_function_exists(gettimeofday  CF_HAVE_GETTIMEOFDAY)

#######################################################################################
# Win32 specific
#######################################################################################

if(WIN32)

  coolfluid_log( "+++++  Checking for the Win32 windows.h header" )    # check windows.hfor Windows API
  check_include_file_cxx(windows.h CF_HAVE_WINDOWSH)
  coolfluid_log( "+++++  Checking for the Win32 dbghelp.h header" )    # check dbghelp.h for call stack
  check_include_file_cxx(dbghelp.h CF_HAVE_DBGHELPH)
  coolfluid_log( "+++++  Checking for the Win32 psapi.h header" )      # check psapi.h for memory info
  check_include_file_cxx(psapi.h CF_HAVE_PSAPIH)

endif()

#######################################################################################
# UNIX specific
#######################################################################################

if(UNIX)

  coolfluid_log( "+++++  Checking for the dlfcn.h header" )  # for dlopen
  check_include_file_cxx(dlfcn.h CF_HAVE_DLOPEN)

endif()

