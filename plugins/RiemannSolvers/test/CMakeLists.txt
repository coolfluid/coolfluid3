
coolfluid_add_test( UTEST   utest-riemannsolver
                    CPP     utest-riemannsolver.cpp
                    PLUGINS Physics
                    LIBS    coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler )

coolfluid_add_test( UTEST   utest-riemannsolvers-ausmplusup
                    CPP     utest-riemannsolvers-ausmplusup.cpp
                    PLUGINS Physics
                    LIBS    coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler )

coolfluid_add_test( UTEST   utest-riemannsolvers-central
                    CPP     utest-riemannsolvers-central.cpp
                    PLUGINS Physics
                    LIBS    coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler )

coolfluid_add_test( UTEST   utest-riemannsolvers-laxfriedrich
                    CPP     utest-riemannsolvers-laxfriedrich.cpp
                    PLUGINS Physics
                    LIBS    coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler )
