coolfluid_find_orphan_files()

list( APPEND coolfluid_actions_files
  CAction.hpp
  CAction.cpp
  CDummyLoopOperation.hpp
  CDummyLoopOperation.cpp
  CLoopOperation.hpp
  CLoopOperation.cpp
  CForAllElementsT.hpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllNodes.hpp
  CForAllNodes.cpp
  CLoop.hpp
  CLoop.cpp
  CSchemeLDA.hpp
  CSchemeLDA.cpp
  CSchemeLDAT.hpp
  CSetFieldValues.hpp
  CSetFieldValues.cpp
  CTakeStep.hpp
  CTakeStep.cpp
  LibActions.hpp
  LibActions.cpp
)

option( CF_ENABLE_PROTO            "Build boost::proto related code for expressions"               ON )

if(CF_ENABLE_PROTO)
  list( APPEND coolfluid_actions_files 
    Proto/CProtoElementsAction.hpp
    Proto/ProtoContexts.hpp
    Proto/ProtoDomain.hpp
    Proto/ProtoEigenContext.hpp
    Proto/ProtoElementData.hpp
    Proto/ProtoElementLooper.hpp
    Proto/ProtoFunctions.hpp
    Proto/ProtoGrammars.hpp
    Proto/ProtoNodeContexts.hpp
    Proto/ProtoNodeLooper.hpp
    Proto/ProtoSFContexts.hpp
    Proto/ProtoTransforms.hpp
    Proto/ProtoVariables.hpp
  )
else()
  coolfluid_mark_not_orphan(
    Proto/CProtoElementsAction.hpp
    Proto/ProtoContexts.hpp
    Proto/ProtoDomain.hpp
    Proto/ProtoElementData.hpp
    Proto/ProtoElementLooper.hpp
    Proto/ProtoFunctions.hpp
    Proto/ProtoGrammars.hpp
    Proto/ProtoNodeContexts.hpp
    Proto/ProtoNodeLooper.hpp
    Proto/ProtoSFContexts.hpp
    Proto/ProtoTransforms.hpp
    Proto/ProtoVariables.hpp
  )
endif()

set( coolfluid_actions_kernellib TRUE )

list( APPEND coolfluid_actions_cflibs coolfluid_mesh )

coolfluid_add_library( coolfluid_actions )

add_subdirectory(uTests)
