list( APPEND coolfluid-mesh-transformer_files 
        coolfluid-mesh-transformer.cpp
        Help.hpp
        Help.cpp
        Transformer.hpp
        Transformer.cpp 
    )

list( APPEND coolfluid-mesh-transformer_cflibs ${CF3_KERNEL_LIBS} coolfluid_shell)

coolfluid_add_application( coolfluid-mesh-transformer )
