coolfluid3_add_library( TARGET coolfluid_zoltan
  SOURCES
    LibZoltan.hpp
    LibZoltan.cpp
    PHG.hpp
    PHG.cpp
  LIBS
    coolfluid_mesh
    ${ZOLTAN_LIBRARIES}
    ${ZOLTAN_EXTRA_LIBRARIES}
  INCLUDES
    ${ZOLTAN_INCLUDE_DIRS}
    ${ZOLTAN_EXTRA_INCLUDE_DIRS})
