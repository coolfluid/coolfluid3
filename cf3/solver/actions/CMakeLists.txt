coolfluid_find_orphan_files()

list( APPEND coolfluid_solver_actions_files
  AdvanceTime.hpp
  AdvanceTime.cpp
  Criterion.hpp
  Criterion.cpp
  CriterionAbsResidual.hpp
  CriterionAbsResidual.cpp
  CriterionMaxIterations.hpp
  CriterionMaxIterations.cpp
  CriterionMilestoneIteration.hpp
  CriterionMilestoneIteration.cpp
  CriterionMilestoneTime.hpp
  CriterionMilestoneTime.cpp
  CriterionTime.hpp
  CriterionTime.cpp
  Iterate.hpp
  Iterate.cpp
  LoopOperation.hpp
  LoopOperation.cpp
  Probe.hpp
  Probe.cpp
  ProbePostProcFunction.hpp
  ProbePostProcFunction.cpp
  ProbePostProcHistory.hpp
  ProbePostProcHistory.cpp
  ForAllCells.hpp
  ForAllCells.cpp
  ForAllElements.hpp
  ForAllElements.cpp
  ForAllElementsT.hpp
  ForAllNodes2.hpp
  ForAllNodes2.cpp
  ForAllFaces.hpp
  ForAllFaces.cpp
  Loop.hpp
  Loop.cpp
  PrintIterationSummary.hpp
  PrintIterationSummary.cpp
  SynchronizeFields.hpp
  SynchronizeFields.cpp
  ComputeArea.hpp
  ComputeArea.cpp
  ComputeVolume.hpp
  ComputeVolume.cpp
  ComputeLNorm.hpp
  ComputeLNorm.cpp
  PeriodicWriteMesh.hpp
  PeriodicWriteMesh.cpp
  LibActions.hpp
  LibActions.cpp
  Conditional.hpp
  Conditional.cpp
)

list( APPEND coolfluid_solver_actions_proto_files
    Proto/BlockAccumulator.hpp
    Proto/ComponentWrapper.hpp
    Proto/ConfigurableConstant.hpp
    Proto/FieldSync.hpp
    Proto/FieldSync.cpp
    Proto/ProtoAction.hpp
    Proto/ProtoAction.cpp
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
    Proto/GaussPoints.hpp
    Proto/IndexLooping.hpp
    Proto/LSSWrapper.hpp
    Proto/NeumannBC.hpp
    Proto/NodeData.hpp
    Proto/NodeGrammar.hpp
    Proto/NodeLooper.hpp
    Proto/PhysicsConstant.hpp
    Proto/RestrictExpressionToElementType.hpp
    Proto/SolutionVector.hpp
    Proto/Terminals.hpp
    Proto/Transforms.hpp
)

if( CF3_ENABLE_PROTO )
  list( APPEND coolfluid_solver_actions_files ${coolfluid_solver_actions_proto_files} )
else()
  coolfluid_mark_not_orphan( ${coolfluid_solver_actions_proto_files} )
endif()

set( coolfluid_solver_actions_kernellib TRUE )

list( APPEND coolfluid_solver_actions_cflibs coolfluid_solver coolfluid_math_lss coolfluid_mesh )

coolfluid_add_library( coolfluid_solver_actions )
