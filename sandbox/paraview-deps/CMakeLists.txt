if( QT_FOUND AND ParaView_FOUND )

  include(CMake/ParaViewUse)

  list( APPEND sandbox-paraview-deps_libs ${QT_LIBRARIES} ${ParaView_LIBRARIES} )
  list( APPEND sandbox-paraview-deps_files paraview-deps.cpp )

  set( sandbox-paraview-deps_app TRUE )

  coolfluid_add_application( sandbox-paraview-deps )
else()
  coolfluid_mark_not_orphan( paraview-deps.cpp )
endif()
