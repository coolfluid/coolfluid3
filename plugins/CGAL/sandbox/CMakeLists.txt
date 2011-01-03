coolfluid_find_orphan_files()

option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

list( APPEND sandbox-bounding-ellipse_files
  linear_interpolation.cpp
)

list( APPEND sandbox-bounding-ellipse_includedirs ${CGAL_INCLUDE_DIR} )
list( APPEND sandbox-bounding-ellipse_libs ${CGAL_LIBRARIES} ${Boost_THREAD_LIBRARY} )
list( APPEND sandbox-bounding-ellipse_cflibs coolfluid_mesh )

set( sandbox-bounding-ellipse_sandbox_app TRUE )
set( sandbox-bounding-ellipse_condition ${CF_HAVE_CGAL} )

coolfluid_add_application( sandbox-bounding-ellipse )

#avoid a conflict between boost and CGAL
set(sandbox-bounding-ellipse_COMPILE_FLAGS "-frounding-math -DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")
if(CF_STRIP_CGAL_COMPILE_FLAGS)
  set(sandbox-bounding-ellipse_COMPILE_FLAGS "${CGAL_CF_COMPILE_FLAGS} -g0 -O0")
endif()

set_source_files_properties(linear_interpolation.cpp
    PROPERTIES COMPILE_FLAGS ${sandbox-bounding-ellipse_COMPILE_FLAGS})
