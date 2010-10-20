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
  ProtoContexts.hpp
  ProtoDomain.hpp
  ProtoElementLooper.hpp
  ProtoFunctions.hpp
  ProtoGrammars.hpp
  ProtoNodeContexts.hpp
  ProtoNodeLooper.hpp
  ProtoSFContexts.hpp
  ProtoTransforms.hpp
  ProtoVariables.hpp
)

set( coolfluid_actions_kernellib TRUE )

list( APPEND coolfluid_actions_cflibs coolfluid_mesh )

coolfluid_add_library( coolfluid_actions )

add_subdirectory(uTests)
