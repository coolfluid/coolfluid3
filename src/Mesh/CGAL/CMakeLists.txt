IF(CF_HAVE_CGAL AND src_mesh_CGAL_enabled)

option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

LIST( APPEND CGAL_CF_files
  CGALAPI.hpp
  ImplicitFunctionMesh.cpp
  ImplicitFunctionMesh.hpp
)

LIST( APPEND CGAL_CF_includedirs ${CGAL_INCLUDE_DIR} )
LIST( APPEND CGAL_CF_libs
                ${CGAL_LIBRARY}
                ${Boost_THREAD_LIBRARY} )
LIST( APPEND CGAL_CF_cflibs Mesh )
SET( CGAL_kernellib TRUE )

CF_ADD_LIBRARY( CGAL_CF)
#avoid a conflict between boost and CGAL
SET(CGAL_CF_COMPILE_FLAGS "-frounding-math -DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")
IF(CF_STRIP_CGAL_COMPILE_FLAGS)
  SET(CGAL_CF_COMPILE_FLAGS "${CGAL_CF_COMPILE_FLAGS} -g0 -O0")
ENDIF()

SET_SOURCE_FILES_PROPERTIES(ImplicitFunctionMesh.cpp 
    PROPERTIES COMPILE_FLAGS ${CGAL_CF_COMPILE_FLAGS})
add_subdirectory( uTests )

ELSE()

  CF_MARK_NOT_ORPHAN(
    CGALAPI.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/uTestTetraSF.cpp  )

ENDIF()
