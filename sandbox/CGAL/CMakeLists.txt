CF_FIND_ORPHAN_FILES()

IF(CF_HAVE_CGAL)

option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

LIST( APPEND bounding-ellipse_files
  linear_interpolation.cpp
)

LIST( APPEND bounding-ellipse_includedirs ${CGAL_INCLUDE_DIR} )
LIST( APPEND bounding-ellipse_libs ${CGAL_LIBRARY} ${Boost_THREAD_LIBRARY} )
LIST( APPEND bounding-ellipse_cflibs Mesh )

CF_ADD_APP( bounding-ellipse )
#avoid a conflict between boost and CGAL
SET(bounding-ellipse_COMPILE_FLAGS "-frounding-math -DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")
IF(CF_STRIP_CGAL_COMPILE_FLAGS)
  SET(bounding-ellipse_COMPILE_FLAGS "${CGAL_CF_COMPILE_FLAGS} -g0 -O0")
ENDIF()

SET_SOURCE_FILES_PROPERTIES(linear_interpolation.cpp 
    PROPERTIES COMPILE_FLAGS ${bounding-ellipse_COMPILE_FLAGS})

ELSE()

  CF_MARK_NOT_ORPHAN (
      linear_interpolation.cpp
  )

ENDIF()
