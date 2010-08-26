if( CF_HAVE_CGAL )

  option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

  list( APPEND CF_CGAL_files
    CGALAPI.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
  )

  list( APPEND CF_CGAL_includedirs ${CGAL_INCLUDE_DIR} )
  list( APPEND CF_CGAL_libs
                ${CGAL_LIBRARY}
                ${Boost_THREAD_LIBRARY} )
  list( APPEND CF_CGAL_cflibs Mesh )
  set( CF_CGAL_kernellib TRUE )

  # avoid a conflict between boost and CGAL
  ADD_DEFINITIONS(-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE)

  CF_ADD_LIBRARY( CF_CGAL )

  set( CF_CGAL_COMPILE_FLAGS "" )
  if( CMAKE_COMPILER_IS_GNUCC )

    set(CF_CGAL_COMPILE_FLAGS "${CF_CGAL_COMPILE_FLAGS} -frounding-math")

    # just for this library, disable optimizations
    # to avoid enormous compilation times and memory usage
    if(CF_STRIP_CGAL_COMPILE_FLAGS)
      set(CF_CGAL_COMPILE_FLAGS "${CF_CGAL_COMPILE_FLAGS} -g0 -O0")
    endif(CF_STRIP_CGAL_COMPILE_FLAGS)

    SET_SOURCE_FILES_PROPERTIES(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${CF_CGAL_COMPILE_FLAGS})
  
  endif( CMAKE_COMPILER_IS_GNUCC )

add_subdirectory( uTests )

else()

  CF_MARK_NOT_ORPHAN(
    CGALAPI.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/uTestTetraSF.cpp  )

endif()
