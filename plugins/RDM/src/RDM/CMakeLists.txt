list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  RKRD.cpp
  RKRD.hpp
  SupportedTypes.hpp
# wizards
  ScalarAdvection.hpp
  ScalarAdvection.cpp
# actions
  Action.hpp
  Action.cpp
  BoundaryTerm.hpp
  BoundaryTerm.cpp
  DomainTerm.hpp
  DomainTerm.cpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  Cleanup.hpp
  Cleanup.cpp
# time stepping
  ForwardEuler.hpp
  ForwardEuler.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
# schemes
# --  FluxOp2D.hpp
# --  SysFluxOp2D.hpp
# --  CSysFluxOp2D.hpp
# --  LDA.hpp
# --  LDA.cpp
# --  SysLDA.hpp
# --  SysLDA.cpp
  CSysLDA.hpp
  CSysLDA.cpp
  CSysLF.hpp
  CSysLF.cpp
 # Blended.hpp
 # Blended.cpp
 # SUPG.hpp
 # SUPG.cpp
# --  SchemeLDA.hpp
# --  SchemeSysLDA.hpp
  SchemeBase.hpp
  SchemeCSysLDA.hpp
  SchemeCSysLF.hpp
# --  SchemeN.hpp
# --  SchemeB.hpp
# --  SchemeSUPG.hpp
# physical models
  Euler2D.hpp
  Euler2D.cpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  LinearAdvSys2D.hpp
  LinearAdvSys2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
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


list( APPEND coolfluid_rdm_gpu_files 
          LDAGPU.hpp
          LDAGPU.cpp
          SchemeLDAGPU.hpp
          CLdeclaration.hpp
          LDAGPUkernel.hpp         
)
   
if(CF_ENABLE_GPU AND OPENCL_FOUND)

  coolfluid_mark_not_orphan(${coolfluid_rdm_gpu_files})

#  list( APPEND coolfluid_rdm_files ${coolfluid_rdm_gpu_files})
#  list( APPEND coolfluid_rdm_includedirs ${OPENCL_INCLUDE_DIR} )
#  list( APPEND coolfluid_rdm_libs   ${OPENCL_LIBRARIES} )
else()
  coolfluid_mark_not_orphan(${coolfluid_rdm_gpu_files})
endif()

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
