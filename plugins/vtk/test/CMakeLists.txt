coolfluid_add_test( UTEST       utest-vtk-tetrahedralize
                    PYTHON      utest-vtk-tetrahedralize.py
                    CONDITION   coolfluid_vtk_builds )

coolfluid_add_test( UTEST       utest-vtk-meshinterpolator
                    PYTHON      utest-vtk-meshinterpolator.py
                    MPI         8
                    CONDITION   coolfluid_vtk_builds )