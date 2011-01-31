coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_actions_files
  CNodeOperation.hpp
  CNodeOperation.cpp
  CLoopOperation.hpp
  CLoopOperation.cpp
  CForAllT.hpp
  CForAllElementsT.hpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllElements2.hpp
  CForAllElements2.cpp
  CForAllNodes.hpp
  CForAllNodes.cpp
  CForAllNodes2.hpp
  CForAllNodes2.cpp
  CForAllFaces.hpp
  CForAllFaces.cpp
  CLoop.hpp
  CLoop.cpp
  CSetFieldValues.hpp
  CSetFieldValues.cpp
  CSetFieldValues2.hpp
  CSetFieldValues2.cpp
  CComputeVolume.hpp
  CComputeVolume.cpp
  LibActions.hpp
  LibActions.cpp
)

option( CF_ENABLE_PROTO            "Build boost::proto related code for expressions"               ON )

if(CF_ENABLE_PROTO)
  list( APPEND coolfluid_solver_actions_files 
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
    Proto/ExpressionGroup.hpp
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
    Proto/ExpressionGroup.hpp
    Proto/Functions.hpp
    Proto/NeumannBC.hpp
    Proto/NodeData.hpp
    Proto/NodeGrammar.hpp
    Proto/NodeLooper.hpp
    Proto/Terminals.hpp
    Proto/Transforms.hpp
  )
endif()

set( coolfluid_solver_actions_kernellib TRUE )

list( APPEND coolfluid_solver_actions_cflibs coolfluid_solver coolfluid_mesh )

coolfluid_add_library( coolfluid_solver_actions )
