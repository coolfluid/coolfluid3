coolfluid3_add_library( TARGET coolfluid_cgal
            SOURCES
              LibCGAL.cpp
              LibCGAL.hpp
              ImplicitFunctionMesh.cpp
              ImplicitFunctionMesh.hpp
              Tetrahedralize.hpp
              Tetrahedralize.cpp
            LIBS
              coolfluid_mesh_lagrangep1
              ${CGAL_LIBRARY}
              ${Boost_THREAD_LIBRARY}
              ${GMP_LIBRARIES}
              ${MPFR_LIBRARIES}
            INCLUDES
              ${CGAL_INCLUDE_DIR}
              ${GMP_INCLUDE_DIR}
              ${MPFR_INCLUDE_DIR}
            CONDITION  CGAL_FOUND )

set_target_properties(coolfluid_cgal PROPERTIES COMPILE_FLAGS -Wno-c++11-extensions)

if( coolfluid_cgal_COMPILE_FLAGS )
  set_source_files_properties(ImplicitFunctionMesh.cpp PROPERTIES COMPILE_FLAGS ${coolfluid_cgal_COMPILE_FLAGS})
endif()
