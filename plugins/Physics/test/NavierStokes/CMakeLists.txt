#########################################################################################
# utest-physics-navierstokes-cons2d

list( APPEND utest-physics-navierstokes-cons2d_cflibs coolfluid_physics_navierstokes )
list( APPEND utest-physics-navierstokes-cons2d_files  utest-physics-navierstokes-cons2d.cpp )

coolfluid_add_unit_test( utest-physics-navierstokes-cons2d )

#########################################################################################

