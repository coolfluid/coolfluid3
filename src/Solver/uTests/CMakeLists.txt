
list( APPEND utest-cphysical-model_cflibs coolfluid_rdm )
list( APPEND utest-cphysical-model_files
  uTestCPhysicalModel.cpp )

coolfluid_add_unittest( utest-cphysical-model )
