#########################################################################
# test physics

list( APPEND utest-physics-variablemanager_cflibs coolfluid_physics )
list( APPEND utest-physics-variablemanager_files  utest-physics-variablemanager.cpp )

coolfluid_add_unit_test( utest-physics-variablemanager )