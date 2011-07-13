list( APPEND coolfluid_rdm_schemes_files
  LibSchemes.cpp
  LibSchemes.hpp
  CSysLDA.hpp
  CSysLDA.cpp
  CSysLF.hpp
  CSysLF.cpp
  CSysN.hpp
  CSysN.cpp
  CSysB.hpp
  CSysB.cpp
  CSysSUPG.hpp
  CSysSUPG.cpp
)

list( APPEND coolfluid_rdm_schemes_cflibs coolfluid_rdm_core )

coolfluid_add_library( coolfluid_rdm_schemes )
