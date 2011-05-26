list( APPEND sandbox-lss-trilinos_files
  lss-trilinos.cpp
  lss-interface.hpp
  blockaccumulator.hpp
)
list( APPEND sandbox-lss-trilinos_libs  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

set( sandbox-lss-trilinos_sandbox_app TRUE )

coolfluid_add_application( sandbox-lss-trilinos )
