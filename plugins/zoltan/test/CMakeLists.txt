if(coolfluid_zoltan_builds)

coolfluid_add_test( UTEST       utest-zoltan-phg
                    PYTHON      utest-zoltan-phg.py
                    MPI 4)
                    
endif()