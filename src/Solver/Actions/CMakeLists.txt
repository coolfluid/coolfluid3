coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_actions_files
  CAdvanceTime.hpp
  CAdvanceTime.cpp
  CCriterion.hpp
  CCriterion.cpp
  CCriterionAbsResidual.hpp
  CCriterionAbsResidual.cpp
  CCriterionMaxIterations.hpp
  CCriterionMaxIterations.cpp
  CCriterionMilestoneIteration.hpp
  CCriterionMilestoneIteration.cpp
  CCriterionMilestoneTime.hpp
  CCriterionMilestoneTime.cpp
  CCriterionTime.hpp
  CCriterionTime.cpp
  CIterate.hpp
  CIterate.cpp
  CLoopOperation.hpp
  CLoopOperation.cpp
  CForAllCells.hpp
  CForAllCells.cpp
  CForAllElements.hpp
  CForAllElements.cpp
  CForAllElementsT.hpp
  CForAllNodes2.hpp
  CForAllNodes2.cpp
  CForAllFaces.hpp
  CForAllFaces.cpp
  CLoop.hpp
  CLoop.cpp
  CSynchronizeFields.hpp
  CSynchronizeFields.cpp
  CComputeArea.hpp
  CComputeArea.cpp
  CComputeVolume.hpp
  CComputeVolume.cpp
  CComputeLNorm.hpp
  CComputeLNorm.cpp
  CPeriodicWriteMesh.hpp
  CPeriodicWriteMesh.cpp
  CSolveSystem.hpp
  CSolveSystem.cpp
  LibActions.hpp
  LibActions.cpp
  Conditional.hpp
  Conditional.cpp
)

list( APPEND coolfluid_solver_actions_proto_files
    Proto/BlockAccumulator.hpp
    Proto/ConfigurableConstant.hpp
    Proto/CProtoAction.hpp
    Proto/CProtoAction.cpp
    Proto/DirichletBC.hpp
    Proto/EigenTransforms.hpp
    Proto/ElementData.hpp
    Proto/ElementExpressionWrapper.hpp
    Proto/ElementGrammar.hpp
    Proto/ElementIntegration.hpp
    Proto/ElementLooper.hpp
    Proto/ElementMatrix.hpp
    Proto/ElementOperations.hpp
    Proto/ElementTransforms.hpp
    Proto/Expression.hpp
    Proto/ExpressionGroup.hpp
    Proto/ForEachDimension.hpp
    Proto/Functions.hpp
    Proto/IndexLooping.hpp
    Proto/LSSProxy.hpp
    Proto/LSSProxy.cpp
    Proto/NeumannBC.hpp
    Proto/NodeData.hpp
    Proto/NodeGrammar.hpp
    Proto/NodeLooper.hpp
    Proto/SolutionVector.hpp
    Proto/Terminals.hpp
    Proto/Transforms.hpp
)

option( CF_ENABLE_PROTO            "Build boost::proto related code for expressions"               ON )

coolfluid_set_feature( Proto CF_ENABLE_PROTO "boost proto expressions" )

if( CF_ENABLE_PROTO )
  list( APPEND coolfluid_solver_actions_files ${coolfluid_solver_actions_proto_files} )
else()
  coolfluid_mark_not_orphan( ${coolfluid_solver_actions_proto_files} )
endif()

set( coolfluid_solver_actions_kernellib TRUE )

list( APPEND coolfluid_solver_actions_cflibs coolfluid_solver )

coolfluid_add_library( coolfluid_solver_actions )
