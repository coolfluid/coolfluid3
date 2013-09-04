coolfluid_add_test( ATEST atest-ufem-demo-poisson
                    PYTHON atest-ufem-demo-poisson.py)

coolfluid_add_test( ATEST atest-ufem-demo-all
                    PYTHON atest-ufem-demo-all.py)

coolfluid_add_test( PTEST ptest-ufem-demo-assembly
                    PYTHON ptest-ufem-demo-assembly.py)
                    
coolfluid_add_test( PTEST ptest-ufem-demo-navier-stokes
                    PYTHON ptest-ufem-demo-navier-stokes.py)

coolfluid_add_test( ATEST atest-ufem-demo-chorin
                    PYTHON atest-ufem-demo-chorin.py)