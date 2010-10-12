coolfluid_find_orphan_files()

list( APPEND Actions_files
  CAction.hpp
  CAction.cpp
  CElementOperation.hpp
  CElementOperation.cpp
  CForAllElementsT.hpp
  CForAllElements.hpp
  CForAllElements.cpp
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
)

set( Actions_kernellib TRUE )

list( APPEND Actions_cflibs Mesh )

coolfluid_add_library(Actions)

add_subdirectory(uTests)