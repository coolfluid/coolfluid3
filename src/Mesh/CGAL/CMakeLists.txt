IF(CF_HAVE_CGAL)

LIST( APPEND CGAL_files
  CGALAPI.hpp
  ImplicitFunctionMesh.cpp
  ImplicitFunctionMesh.hpp
)

LIST( APPEND CGAL_includedirs ${CGAL_INCLUDE_DIR} )
LIST( APPEND CGAL_libs
                ${CGAL_LIBRARY}
                ${Boost_THREAD_LIBRARY} )
LIST( APPEND CGAL_cflibs Mesh )
SET( CGAL_kernellib TRUE )

CF_ADD_LIBRARY( CGAL )
#avoid a conflict between boost and CGAL
SET_SOURCE_FILES_PROPERTIES(ImplicitFunctionMesh.cpp 
    PROPERTIES COMPILE_FLAGS "-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")
add_subdirectory( uTests )

ELSE()

  CF_MARK_NOT_ORPHAN (
    CGALAPI.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/uTestTetraSF.cpp  )

ENDIF()
