coolfluid_find_orphan_files()

list( APPEND Actions_files
  CAction.hpp
  CAction.cpp
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

set( Actions_kernellib TRUE )

list( APPEND Actions_cflibs Mesh )

coolfluid_add_library(Actions)

add_subdirectory(uTests)