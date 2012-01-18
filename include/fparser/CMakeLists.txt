# parser library

coolfluid_find_orphan_files()

#if( CF3_HAVE_MATH_TR1_ASINH )
#  add_definitions( -DFP_SUPPORT_ASINH )
#endif()
#if( CF3_HAVE_MATH_TR1_EXP2 )
#  add_definitions( -DFP_SUPPORT_EXP2 )
#endif()
#if( CF3_HAVE_MATH_TR1_LOG2 )
#  add_definitions( -DFP_SUPPORT_LOG2 )
#endif()
#if( CF3_HAVE_MATH_TR1_CBRT )
#  add_definitions( -DFP_SUPPORT_CBRT )
#endif()
#if( CF3_HAVE_MATH_TR1_HYPOT )
#  add_definitions( -DFP_SUPPORT_HYPOT )
#endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

list( APPEND coolfluid_fparser_files
  LibFParser.hpp
  fpconfig.hh
  fparser.hh
  fparser.cc
  fpoptimizer.cc
  extrasrc/fptypes.hh
  extrasrc/fpaux.hh
  extrasrc/fp_opcode_add.inc
  extrasrc/fp_identifier_parser.inc
)

coolfluid_mark_not_orphan(
  fparser_mpfr.hh
  fparser_gmpint.hh
  mpfr/GmpInt.hh
  mpfr/GmpInt.cc
  mpfr/MpfrFloat.hh
  mpfr/MpfrFloat.cc
)

if(CF3_HAVE_CMATH)
  list( APPEND coolfluid_fparser_libs ${CMATH_LIBRARIES} )
endif()
set( coolfluid_fparser_kernellib TRUE )

coolfluid_add_library( coolfluid_fparser )

# first example
coolfluid_add_test( UTEST utest-fparser-1
                    CPP   examples/example.cc
                    LIBS  coolfluid_fparser
                    CONDITION OFF )

# second example
coolfluid_add_test( UTEST utest-fparser-2
                    CPP   examples/example2.cc
                    LIBS  coolfluid_fparser
                    CONDITION OFF )

