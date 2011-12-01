list( APPEND coolfluid_cgal_files
    LibCGAL.cpp
    LibCGAL.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
)

find_package(CGAL)
find_package(GMP)
find_package(MPFR)

list( APPEND coolfluid_cgal_includedirs ${CGAL_INCLUDE_DIR} )
list( APPEND coolfluid_cgal_libs        ${CGAL_LIBRARIES} ${Boost_THREAD_LIBRARY} )
list( APPEND coolfluid_cgal_cflibs      coolfluid_mesh_lagrangep1 )

if(GMP_FOUND)
  list( APPEND coolfluid_cgal_includedirs ${GMP_INCLUDE_DIR} )
  list( APPEND coolfluid_cgal_libs        ${GMP_LIBRARIES})
endif()

if(MPFR_FOUND)
  list( APPEND coolfluid_cgal_includedirs ${MPFR_INCLUDE_DIR} )
  list( APPEND coolfluid_cgal_libs        ${MPFR_LIBRARIES})
endif()

set( coolfluid_cgal_condition  ${CF3_HAVE_CGAL} )

coolfluid_add_library( coolfluid_cgal )

if( coolfluid_cgal_COMPILE_FLAGS )
  set_source_files_properties(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${coolfluid_cgal_COMPILE_FLAGS})
endif()
