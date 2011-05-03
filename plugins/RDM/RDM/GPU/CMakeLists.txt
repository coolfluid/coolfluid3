########################################################################
# coolfluid_rdm_gpu

list( APPEND coolfluid_rdm_gpu_files
  # library
  LibGPU.cpp
  LibGPU.hpp
          LDAGPU.hpp
          LDAGPU.cpp
          SchemeLDAGPU.hpp
          CLdeclaration.hpp
          LDAGPUkernel.hpp   
          CSysLDAGPU.hpp
          SchemeCSysLDAGPU.hpp
          CSysLDAGPU.cpp
          sysLDAGPUkernel.hpp
                
)
   
if(CF_ENABLE_GPU AND OPENCL_FOUND)
  set( RDM_GPU 1 )
else()
  set( RDM_GPU 0 )
endif()

list( APPEND coolfluid_rdm_gpu_cflibs      coolfluid_rdm_schemes )
list( APPEND coolfluid_rdm_gpu_condition   ${RDM_GPU} )
list( APPEND coolfluid_rdm_gpu_includedirs ${OPENCL_INCLUDE_DIR} )
list( APPEND coolfluid_rdm_gpu_libs        ${OPENCL_LIBRARIES}   )

coolfluid_add_library( coolfluid_rdm_gpu )

