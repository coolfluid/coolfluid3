# parser library

if( CF_HAVE_MATH_ASINH )
  add_definitions( -DFP_SUPPORT_ASINH )
endif()
if( CF_HAVE_MATH_EXP2 )
  add_definitions( -DFP_SUPPORT_EXP2 )
endif()
if( CF_HAVE_MATH_LOG2 )
  add_definitions( -DFP_SUPPORT_LOG2 )
endif()
if( CF_HAVE_MATH_CBRT )
  add_definitions( -DFP_SUPPORT_CBRT )
endif()
if( CF_HAVE_MATH_HYPOT )
  add_definitions( -DFP_SUPPORT_HYPOT )
endif()

list( APPEND fparser_files 
  LibFParser.hpp
  fpconfig.hh
  fptypes.hh
  fparser.hh
  fparser.cc
	fpaux.hh                 
	fpoptimizer.cc
	fp_opcode_add.inc        
	fp_identifier_parser.inc
#	fparser_mpfr.hh
#	fparser_gmpint.hh
# mpfr/GmpInt.hh
# mpfr/GmpInt.cc
# mpfr/MpfrFloat.hh
# mpfr/MpfrFloat.cc
)

list( APPEND fparser_libs ${CMATH_LIBRARIES} )
set( fparser_kernellib TRUE )

coolfluid_add_library( fparser )

# first example
# list( APPEND utest-fparser-1_files examples/example.cc )
# list( APPEND utest-fparser-1_cflibs fparser )

# coolfluid_add_unittest( utest-fparser-1 )

# second example
# list( APPEND utest-fparser-2_files examples/example2.cc )
# list( APPEND utest-fparser-2_cflibs fparser )

# coolfluid_add_unittest( utest-fparser-2 )
