##############################################################################
# this macro separates the sources form the headers
##############################################################################
macro( coolfluid_add_c_flags m_c_flags )

  if( NOT DEFINED N_CFLAG )
    set( N_CFLAG 0 )
  endif()

 	MATH ( EXPR N_CFLAG '${N_CFLAG}+1'  )    

  CHECK_C_COMPILER_FLAG ( ${m_c_flags} C_FLAG_TEST_${N_CFLAG} )

#  message( STATUS "FLAG [${m_c_flags}] is [${C_FLAG_TEST_${N_CFLAG}}] " )
  if( C_FLAG_TEST_${N_CFLAG} )
	  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${m_c_flags}" )
    coolfluid_log( "C FLAG [${m_c_flags}] added" )
  ELSE  ()
    coolfluid_log( "C FLAG [${m_c_flags}] skipped" )
  endif()

endmacro( coolfluid_add_c_flags )
##############################################################################

##############################################################################
macro( coolfluid_add_cxx_flags m_cxx_flags )

  if( NOT DEFINED N_CXXFLAG )
    set( N_CXXFLAG 0 )
  endif()

 	MATH ( EXPR N_CXXFLAG '${N_CXXFLAG}+1'  )    

  CHECK_CXX_COMPILER_FLAG ( ${m_cxx_flags} CXX_FLAG_TEST_${N_CXXFLAG} )

#  message( STATUS "FLAG [${m_cxx_flags}] is [${CXX_FLAG_TEST_${N_CXXFLAG}}] " )
  if( CXX_FLAG_TEST_${N_CXXFLAG} )
	  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${m_cxx_flags}" )
    coolfluid_log( "C++ FLAG [${m_cxx_flags}] added" )
  ELSE  ()
    coolfluid_log( "C++ FLAG [${m_cxx_flags}] skipped" )
  endif()

endmacro( coolfluid_add_cxx_flags )
##############################################################################

##############################################################################
macro( coolfluid_add_c_flags_SIGNAL_ERROR m_c_flags )

  coolfluid_add_c_flags ( ${m_c_flags}  )
  if( NOT C_FLAG_TEST_${N_CFLAG} )
    MESSAGE(FATAL_ERROR "C compiler [${CMAKE_C_COMPILER}] cannot used requested C flag [${m_c_flags}]")
  endif()

endmacro( coolfluid_add_c_flags_SIGNAL_ERROR )
##############################################################################

##############################################################################
macro( coolfluid_add_cxx_flags_SIGNAL_ERROR m_cxx_flags )

  coolfluid_add_cxx_flags ( ${m_cxx_flags}  )
  if( NOT CXX_FLAG_TEST_${N_CXXFLAG} )
    MESSAGE(FATAL_ERROR "C++ compiler [${CMAKE_CXX_COMPILER}] cannot used requested C++ flag [${m_cxx_flags}]")
  endif()

endmacro( coolfluid_add_cxx_flags_SIGNAL_ERROR )
##############################################################################

