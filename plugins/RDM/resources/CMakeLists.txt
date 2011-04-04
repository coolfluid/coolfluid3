list( APPEND resources_files
	# for RotationAdv2D test cases
  rotation-qd-p1.msh 
  rotation-qd-p2.msh 
  rotation-qd-p3.msh 
  rotation-qd-p4.msh 
  rotation-tg-p1.msh 
  rotation-tg-p2.msh 
  rotation-tg-p3.msh 
  rotation-tg-p4.msh
  # for LinearAdv2D and LinearAdvSys2D test cases
  rectangle2x1-tg-p1-953.msh
  rectangle2x1-tg-p2-3689.msh
  rectangle2x1-qd-p1-861.msh
  rectangle2x1-qd-p2-3321.msh
  # for Burgers2D test case
  square1x1-tg-p1-303n.msh
  square1x1-tg-p1-7614.msh
  square1x1-tg-p2-333n.msh
  square1x1-tg-p2-2kn.msh
  square1x1-qd-p1-256n.msh
  square1x1-qd-p1-1369.msh
  square1x1-qd-p2-289n.msh
  square1x1-tgqd-p1-298n.msh
  # for Euler2D test case
  trapezium1x1-tg-p1-508.msh
  trapezium1x1-tg-p2-1949.msh
  trapezium1x1-tg-p3-4306.msh
)

coolfluid_mark_not_orphan( ${resources_files} )

# copy files to test dir in build tree
foreach( rfile ${resources_files} )

message( "RDM resource file [${rfile}]" )

add_custom_command( OUTPUT  ${rfile}
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${rfile} ${RDM_TEST_RUN_DIR}
                   # COMMENT "Copying file ${rfile} to ${RDM_TEST_RUN_DIR}"
)
endforeach()

add_custom_target( rdm_test_meshes ALL DEPENDS ${resources_files} )
