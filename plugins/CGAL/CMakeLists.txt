if( CF_HAVE_CGAL )

  option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

  list( APPEND coolfluid_cgal_files
    LibCGAL.cpp
    LibCGAL.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
  )

  list( APPEND coolfluid_cgal_includedirs ${CGAL_INCLUDE_DIR} )
  list( APPEND coolfluid_cgal_libs        ${CGAL_LIBRARY} ${Boost_THREAD_LIBRARY} )
  list( APPEND coolfluid_cgal_cflibs coolfluid_mesh )

  set( coolfluid_cgal_kernellib TRUE )

  # avoid a conflict between boost and CGAL
  add_definitions(-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE)

  coolfluid_add_library( coolfluid_cgal )

  set( coolfluid_cgal_COMPILE_FLAGS "" )
  if( CMAKE_COMPILER_IS_GNUCC )

    set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -frounding-math")

    # just for this library, disable optimizations
    # to avoid enormous compilation times and memory usage
    if(CF_STRIP_CGAL_COMPILE_FLAGS)
      set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -g0 -O0")
    endif()

    set_source_files_properties(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${coolfluid_cgal_COMPILE_FLAGS})

  endif()

add_subdirectory( uTests )

else()

  coolfluid_mark_not_orphan(
    LibCGAL.hpp
    LibCGAL.cpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
    uTests/utest-tetra-sf.cpp  )

endif()
