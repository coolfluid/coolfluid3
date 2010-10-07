
list( APPEND uTestCPhysicalModel_cflibs Solver )
list( APPEND uTestCPhysicalModel_files
  uTestCPhysicalModel.cpp )

coolfluid_add_unittest( uTestCPhysicalModel )
