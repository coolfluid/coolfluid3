################################################################################
# Test Client

IF( QT_QTTEST_FOUND )

SET(uTestClient_moc_classes

  NTreeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  CNodeTest.hpp

  # include files to moc from the client

  ../ClientCore.hpp
  ../ClientNetworkComm.hpp
  ../CNode.hpp
  ../NGroup.hpp
  ../NLink.hpp
  ../NLog.hpp
  ../NMesh.hpp
  ../NMethod.hpp
  ../NTree.hpp
  ../TreeNode.hpp
)

QT4_WRAP_CPP(uTestClient_moc_files ${uTestClient_moc_classes})

CONFIGURE_FILE ( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

LIST( APPEND uTestClient_cflibs Common Network)
LIST( APPEND uTestClient_libs ${QT_LIBRARIES})
LIST( APPEND uTestClient_files
  uTestClient.cpp
  ExceptionThrowHandler.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  CNodeTest.hpp
  CNodeTest.cpp
  MyNode.cpp
  MyNode.hpp
  NBrowserTest.cpp
  NBrowserTest.hpp
  NLinkTest.cpp
  NLinkTest.hpp
  TreeHandler.cpp
  TreeHandler.hpp

  # include files from the client

  ../ClientCore.cpp
  ../ClientCore.hpp
  ../ClientNetworkComm.cpp
  ../ClientNetworkComm.hpp
  ../ClientRoot.cpp
  ../ClientRoot.hpp
  ../CNode.cpp
  ../CNode.hpp
  ../InvalidValue.cpp
  ../InvalidValue.hpp
  ../NBrowser.cpp
  ../NBrowser.hpp
  ../NGroup.cpp
  ../NGroup.hpp
  ../NLink.cpp
  ../NLink.hpp
  ../NLog.cpp
  ../NLog.hpp
  ../NMesh.cpp
  ../NMesh.hpp
  ../NMethod.cpp
  ../NMethod.hpp
  ../NRoot.cpp
  ../NRoot.hpp
  ../NTree.cpp
  ../NTree.hpp
  ../OptionType.cpp
  ../OptionType.hpp
  ../TObjectProperties.hpp
  ../TreeNode.cpp
  ../TreeNode.hpp
  ../TSshInformation.hpp
  ../UnknownTypeException.cpp
  ../UnknownTypeException.hpp
)

CF_ADD_UNITTEST( uTestClient )

ENDIF()
