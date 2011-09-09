coolfluid_find_orphan_files()

list( APPEND sandbox-lss-trilinos_files
  lss-trilinos.cpp
)

list( APPEND sandbox-lss-trilinos_cflibs coolfluid_common coolfluid_math coolfluid_math_lss)

set( sandbox-lss-trilinos_sandbox_app TRUE )

coolfluid_add_application( sandbox-lss-trilinos )


