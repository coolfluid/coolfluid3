list( APPEND rdm_test_meshes_files
  # for RotationAdv2D test cases
  rotation-qd-p1.neu
  rotation-qd-p1.msh
  rotation-qd-p2.msh
  rotation-qd-p3.msh
  rotation-qd-p4.msh
  rotation-tg-p1.neu
  rotation-tg-p1.msh
  rotation-tg-p2.msh
  rotation-tg-p3.msh
  rotation-tg-p4.msh
  # for LinearAdv2D and LinearAdvSys2D test cases
  rectangle2x1-tg-p1-953.msh
  rectangle2x1-tg-p2-3689.msh
  rectangle2x1-qd-p1-861.msh
  rectangle2x1-qd-p2-3321.msh
   # for LinearAdv3D and LinearAdvSys3D test cases
  box-tet-p1-3112.msh
  box-hexa-p1-1900.msh
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
  trapezium1x1-qd-p1-441.msh
  trapezium1x1-qd-p2-1681.msh
  trapezium1x1-qd-p3-3721.msh
  # for sinus bump test case
  sinusbump-tg-p1-90.msh
  sinusbump-tg-p1-1344.msh
  # for acoustic test case
  circle1d-tg-p1-1451.msh
  # for supersonic ramp test case
  ramp-tg-p1-6506.msh
)

coolfluid_add_resources( rdm_test_meshes )
