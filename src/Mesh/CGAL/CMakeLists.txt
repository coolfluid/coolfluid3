if( CF_HAVE_CGAL )

  option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

  list( APPEND ICGAL_files
    LibCGAL.cpp
    LibCGAL.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
  )

  list( APPEND ICGAL_includedirs ${CGAL_INCLUDE_DIR} )
  list( APPEND ICGAL_libs
                ${CGAL_LIBRARY}
                ${Boost_THREAD_LIBRARY} )
  list( APPEND ICGAL_cflibs Mesh )
  set( ICGAL_kernellib TRUE )

  # avoid a conflict between boost and CGAL
  add_definitions(-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE)

  coolfluid_add_library( ICGAL )

  set( ICGAL_COMPILE_FLAGS "" )
  if( CMAKE_COMPILER_IS_GNUCC )

    set(ICGAL_COMPILE_FLAGS "${ICGAL_COMPILE_FLAGS} -frounding-math")

    # just for this library, disable optimizations
    # to avoid enormous compilation times and memory usage
    if(CF_STRIP_CGAL_COMPILE_FLAGS)
      set(ICGAL_COMPILE_FLAGS "${ICGAL_COMPILE_FLAGS} -g0 -O0")
    endif(CF_STRIP_CGAL_COMPILE_FLAGS)

    set_source_files_properties(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${ICGAL_COMPILE_FLAGS})

  endif( CMAKE_COMPILER_IS_GNUCC )

add_subdirectory( uTests )

else()

  coolfluid_mark_not_orphan(
    LibCGAL.hpp
    LibCGAL.cpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/utest-tetra-sf.cpp  )

endif()
