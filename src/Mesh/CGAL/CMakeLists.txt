IF( CF_HAVE_CGAL )

option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

LIST( APPEND CF_CGAL_files
  CGALAPI.hpp
  ImplicitFunctionMesh.cpp
  ImplicitFunctionMesh.hpp
)

LIST( APPEND CF_CGAL_includedirs ${CGAL_INCLUDE_DIR} )
LIST( APPEND CF_CGAL_libs
                ${CGAL_LIBRARY}
                ${Boost_THREAD_LIBRARY} )
LIST( APPEND CF_CGAL_cflibs Mesh )
SET( CGAL_kernellib TRUE )

CF_ADD_LIBRARY( CF_CGAL )

# Avoid a conflict between boost and CGAL
# TODO: this are compiler specific to GCC
SET(CF_CGAL_COMPILE_FLAGS "-frounding-math -DCF_CGALG_NO_TR1_ARRAY -DCF_CGALG_NO_TR1_TUPLE")
IF(CF_STRIP_CGAL_COMPILE_FLAGS)
  SET(CF_CGAL_COMPILE_FLAGS "${CF_CGAL_COMPILE_FLAGS} -g0 -O0")
ENDIF()

SET_SOURCE_FILES_PROPERTIES(ImplicitFunctionMesh.cpp 
    PROPERTIES COMPILE_FLAGS ${CF_CGAL_COMPILE_FLAGS})
add_subdirectory( uTests )

ELSE()

  CF_MARK_NOT_ORPHAN(
    CGALAPI.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/uTestTetraSF.cpp  )

ENDIF()
