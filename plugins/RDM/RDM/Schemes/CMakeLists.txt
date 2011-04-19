list( APPEND coolfluid_rdm_schemes_files
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
  SchemeBase.hpp
  SchemeCSysN.hpp
  SchemeCSysLDA.hpp
  SchemeCSysB.hpp
  SchemeCSysLF.hpp
  SchemeCSysSUPG.hpp
)

coolfluid_mark_not_orphan(
Blended.hpp
Blended.cpp
SUPG.hpp
SUPG.cpp
FluxOp2D.hpp
SysFluxOp2D.hpp
CSysFluxOp2D.hpp
LDA.hpp
LDA.cpp
SysLDA.hpp
SysLDA.cpp
SchemeLDA.hpp
SchemeSysLDA.hpp
SchemeN.hpp
SchemeB.hpp
SchemeSUPG.hpp
)

list( APPEND coolfluid_rdm_schemes_cflibs coolfluid_rdm_core )

coolfluid_add_library( coolfluid_rdm_schemes )
