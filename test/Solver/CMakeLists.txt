#########################################################################
# test physical model

list( APPEND utest-cphysical-model_cflibs coolfluid_solver )
list( APPEND utest-cphysical-model_files  utest-cphysical-model.cpp )

coolfluid_add_unittest( utest-cphysical-model )

#########################################################################
# test forward euler

list( APPEND utest-forwardeuler_cflibs coolfluid_solver )
list( APPEND utest-forwardeuler_files  utest-forwardeuler.cpp )

coolfluid_add_unittest( utest-forwardeuler )
