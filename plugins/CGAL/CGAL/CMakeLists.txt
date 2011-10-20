list( APPEND coolfluid_cgal_files
    LibCGAL.cpp
    LibCGAL.hpp
    ImplicitFunctionMesh.cpp
    ImplicitFunctionMesh.hpp
)

list( APPEND coolfluid_cgal_includedirs ${CGAL_INCLUDE_DIR} )
list( APPEND coolfluid_cgal_libs        ${CGAL_LIBRARIES} ${Boost_THREAD_LIBRARY} )
list( APPEND coolfluid_cgal_cflibs      coolfluid_mesh_lagrangep1 )

set( coolfluid_cgal_condition  ${CF3_HAVE_CGAL} )

coolfluid_add_library( coolfluid_cgal )

if( coolfluid_cgal_COMPILE_FLAGS )
  set_source_files_properties(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${coolfluid_cgal_COMPILE_FLAGS})
endif()
