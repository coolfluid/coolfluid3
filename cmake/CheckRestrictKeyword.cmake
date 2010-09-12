# This module checks if the C++ compiler supports the restrict keyword or
# some variant of it. The following variants are checked for in that order:
# 1. restrict            (The standard C99 keyword, not yet in C++ standard, Windows VS has it)
# 2. __restrict          (G++ has it)
# 3. __restrict__        (G++ has it too)
# 4. _Restrict           (seems to be used by Sun's compiler)
# These four cases seem to cover all existing variants; however some C++
# compilers don't support any variant, in which case the CF_RESTRICT_KEYWORD variable is set to nothing

set(_CHECK_restrict_KEYWORD_SRC "
char f( const char * restrict x ){  return *x;}
int main(int argc, char *argv[]) { return 0; }
")

set(_CHECK___restrict_KEYWORD_SRC "
char f( const char * __restrict x ){  return *x;}
int main(int argc, char *argv[]) { return 0; }
")

set(_CHECK___restrict___KEYWORD_SRC "
char f( const char * __restrict__ x ){  return *x;}
int main(int argc, char *argv[]) { return 0; }
")

set(_CHECK__Restrict_KEYWORD_SRC "
char f( const char * _Restrict x ) {  return *x; }
int main(int argc, char *argv[]) { return 0; }
")

check_cxx_source_compiles("${_CHECK_restrict_KEYWORD_SRC}"     HAVE_KEYWORD_restrict)
if(HAVE_KEYWORD_restrict)
  set(CF_RESTRICT_KEYWORD restrict)
else()
  check_cxx_source_compiles("${_CHECK___restrict_KEYWORD_SRC}"   HAVE_KEYWORD___restrict)
  if(HAVE_KEYWORD___restrict)
    set(CF_RESTRICT_KEYWORD __restrict)
  else()
    check_cxx_source_compiles("${_CHECK___restrict___KEYWORD_SRC}" HAVE_KEYWORD___restrict__)
    if(HAVE_KEYWORD___restrict__)
      set(CF_RESTRICT_KEYWORD __restrict__)
    else()
      check_cxx_source_compiles("${_CHECK__Restrict_KEYWORD_SRC}"    HAVE_KEYWORD__Restrict)
      if(HAVE_KEYWORD__Restrict)
        set(CF_RESTRICT_KEYWORD _Restrict)
      else()
        set(CF_RESTRICT_KEYWORD) # not supported so keep it empty
      endif()
    endif()
  endif()    
endif()


  