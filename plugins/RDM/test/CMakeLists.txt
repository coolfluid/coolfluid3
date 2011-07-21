# copy test meshes here
set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

##########################################################################

if(CF_ENABLE_GPU AND OPENCL_FOUND)
  set(utest-rdm-rotationadv2dgpu_condition     TRUE)
  set(utest-rdm-translationadv2dgpu_condition  TRUE)
  set(utest-rdm-euler2dgpu_condition           TRUE)
else()
  set(utest-rdm-rotationadv2dgpu_condition     FALSE)
  set(utest-rdm-translationadv2dgpu_condition  FALSE)
  set(utest-rdm-euler2dgpu_condition           FALSE)
endif()

list( APPEND utest-rdm-rotationadv2dgpu_cflibs   coolfluid_rdm_gpu )
list( APPEND utest-rdm-rotationadv2dgpu_files    utest-rdm-rotationadv2d-gpu.cpp )

coolfluid_add_unit_test( utest-rdm-rotationadv2dgpu )

##########################################################################

list( APPEND utest-rdm-translationadv2dgpu_cflibs   coolfluid_rdm_gpu )
list( APPEND utest-rdm-translationadv2dgpu_files    utest-rdm-translationadv2d-gpu.cpp )

coolfluid_add_unit_test( utest-rdm-translationadv2dgpu )

##########################################################################

list( APPEND utest-rdm-euler2dgpu_cflibs   coolfluid_rdm_gpu )
list( APPEND utest-rdm-euler2dgpu_files    utest-rdm-euler2d-gpu.cpp )

coolfluid_add_unit_test( utest-rdm-euler2dgpu )

##########################################################################
# acceptance tests

coolfluid_add_acceptance_test( atest-rdm-linearadv2d atest-rdm-linearadv2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-rotationadv2d atest-rdm-rotationadv2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-burgers2d atest-rdm-burgers2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-euler2d atest-rdm-euler2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-linearadv3d atest-rdm-linearadv3d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-sinusbump atest-rdm-sinusbump.cfscript )

coolfluid_add_acceptance_test( atest-rdm-linearadvsys2d atest-rdm-linearadvsys2d.cfscript )


