coolfluid_find_orphan_files()

list( APPEND coolfluid_actions_files
  CNodeOperation.hpp
  CNodeOperation.cpp
  CLoopOperation.hpp
  CLoopOperation.cpp
  CForAllElementsT.hpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllNodes.hpp
  CForAllNodes.cpp
  CLoop.hpp
  CLoop.cpp
  CSetFieldValues.hpp
  CSetFieldValues.cpp
  LibActions.hpp
  LibActions.cpp
)

option( CF_ENABLE_PROTO            "Build boost::proto related code for expressions"               ON )

if(CF_ENABLE_PROTO)
  list( APPEND coolfluid_actions_files 
    Proto/BlockAccumulator.hpp
    Proto/CProtoElementsAction.hpp
    Proto/CProtoNodesAction.hpp
    Proto/DirichletBC.hpp
    Proto/EigenTransforms.hpp
    Proto/ElementData.hpp
    Proto/ElementGrammar.hpp
    Proto/ElementLooper.hpp
    Proto/ElementTransforms.hpp
    Proto/ElementVariables.hpp
    Proto/Functions.hpp
    Proto/NeumannBC.hpp
    Proto/NodeData.hpp
    Proto/NodeGrammar.hpp
    Proto/NodeLooper.hpp
    Proto/Terminals.hpp
    Proto/Transforms.hpp
  )
else()
  coolfluid_mark_not_orphan(
    Proto/BlockAccumulator.hpp
    Proto/CProtoElementsAction.hpp
    Proto/CProtoNodesAction.hpp
    Proto/DirichletBC.hpp
    Proto/EigenTransforms.hpp
    Proto/ElementData.hpp
    Proto/ElementGrammar.hpp
    Proto/ElementLooper.hpp
    Proto/ElementTransforms.hpp
    Proto/ElementVariables.hpp
    Proto/Functions.hpp
    Proto/NeumannBC.hpp
    Proto/NodeData.hpp
    Proto/NodeGrammar.hpp
    Proto/NodeLooper.hpp
    Proto/Terminals.hpp
    Proto/Transforms.hpp
  )
endif()

set( coolfluid_actions_kernellib TRUE )

list( APPEND coolfluid_actions_cflibs coolfluid_mesh )

coolfluid_add_library( coolfluid_actions )
