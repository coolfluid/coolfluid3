# parser library

list( APPEND fparser_files 
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

list( APPEND fparser_libs ${CMATH_LIBRARY} )
set( fparser_kernellib TRUE )

coolfluid_add_library( fparser )

# first example
list( APPEND utest-fparser-1_files examples/example.cc )
list( APPEND utest-fparser-1_cflibs fparser )

coolfluid_add_unittest( utest-fparser-1 )

# second example
list( APPEND utest-fparser-2_files examples/example2.cc )
list( APPEND utest-fparser-2_cflibs fparser )

coolfluid_add_unittest( utest-fparser-2 )
