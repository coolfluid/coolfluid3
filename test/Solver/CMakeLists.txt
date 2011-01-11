#########################################################################
# test physical model

list( APPEND utest-cphysical-model_cflibs coolfluid_solver )
list( APPEND utest-cphysical-model_files  utest-cphysical-model.cpp )

coolfluid_add_unittest( utest-cphysical-model )
