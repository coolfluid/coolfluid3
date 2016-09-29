coolfluid_add_test( UTEST       utest-vtk-cf3tovtk
										PYTHON      utest-vtk-cf3tovtk.py)

coolfluid_add_test( UTEST       utest-vtk-tetrahedralize
										PYTHON      utest-vtk-tetrahedralize.py)

coolfluid_add_test( UTEST       utest-vtk-meshinterpolator
                    PYTHON      utest-vtk-meshinterpolator.py
										MPI         8)

coolfluid_add_test( UTEST       utest-vtk-multiblockwriter
										PYTHON      utest-vtk-multiblockwriter.py)

coolfluid_add_test( UTEST       utest-vtk-livecoprocessor
										PYTHON      utest-vtk-livecoprocessor.py)
