################################################################################
# Test Client

IF( QT_QTTEST_FOUND )

SET(uTestClient_moc_classes

  NTreeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  CNodeTest.hpp

  # include files to moc from the client

  ../CNode.hpp
  ../ClientNetworkComm.hpp
  ../CommitDetailsDialog.hpp
  ../ConfirmCommitDialog.hpp
  ../ConnectionDialog.hpp
  ../FilesPanel.hpp
  ../LoggingList.hpp
  ../MainWindow.hpp
  ../NCore.hpp
  ../NGroup.hpp
  ../NLink.hpp
  ../NLog.hpp
  ../NMesh.hpp
  ../NMeshReader.hpp
  ../NMethod.hpp
  ../NTree.hpp
  ../OptionPanel.hpp
  ../RemoteFSBrowser.hpp
  ../RemoteSaveFile.hpp
  ../SelectFileDialog.hpp
  ../SelectPathDialog.hpp
  ../StatusModel.hpp
  ../StatusPanel.hpp
  ../TreeNode.hpp
  ../TreeView.hpp
  ../TypeAndNameDialog.hpp
)

QT4_WRAP_CPP(uTestClient_moc_files ${uTestClient_moc_classes})

CONFIGURE_FILE ( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

LIST( APPEND uTestClient_cflibs Network)
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

  ../AboutCFDialog.cpp
  ../AboutCFDialog.hpp
  ../CNode.cpp
  ../CNode.hpp
  ../ClientNetworkComm.cpp
  ../ClientNetworkComm.hpp
  ../ClientRoot.cpp
  ../ClientRoot.hpp
  ../CommitDetails.cpp
  ../CommitDetails.hpp
  ../CommitDetailsDialog.cpp
  ../CommitDetailsDialog.hpp
  ../CommitDetailsItem.cpp
  ../CommitDetailsItem.hpp
  ../ConfirmCommitDialog.cpp
  ../ConfirmCommitDialog.hpp
  ../ConnectionDialog.cpp
  ../ConnectionDialog.hpp
  ../FilesListItem.cpp
  ../FilesListItem.hpp
  ../FilesPanel.cpp
  ../FilesPanel.hpp
  ../GraphicalOption.cpp
  ../GraphicalOption.hpp
  ../LibrariesPanel.cpp
  ../LibrariesPanel.hpp
  ../LoggingList.cpp
  ../LoggingList.hpp
  ../MainWindow.cpp
  ../MainWindow.hpp
  ../MenuActionInfo.cpp
  ../MenuActionInfo.hpp
  ../NBrowser.cpp
  ../NBrowser.hpp
  ../NCore.cpp
  ../NCore.hpp
  ../NGroup.cpp
  ../NGroup.hpp
  ../NLink.cpp
  ../NLink.hpp
  ../NLog.cpp
  ../NLog.hpp
  ../NMesh.cpp
  ../NMesh.hpp
  ../NMeshReader.cpp
  ../NMeshReader.hpp
  ../NMethod.cpp
  ../NMethod.hpp
  ../NRoot.cpp
  ../NRoot.hpp
  ../NTree.cpp
  ../NTree.hpp
  ../OptionPanel.cpp
  ../OptionPanel.hpp
  ../OptionType.cpp
  ../OptionType.hpp
  ../RemoteFSBrowser.cpp
  ../RemoteFSBrowser.hpp
  ../RemoteOpenFile.cpp
  ../RemoteOpenFile.hpp
  ../RemoteSaveFile.cpp
  ../RemoteSaveFile.hpp
  ../SelectFileDialog.cpp
  ../SelectFileDialog.hpp
  ../SelectPathDialog.cpp
  ../SelectPathDialog.hpp
  ../StatusModel.cpp
  ../StatusModel.hpp
  ../StatusPanel.cpp
  ../StatusPanel.hpp
  ../TSshInformation.hpp
  ../TreeNode.cpp
  ../TreeNode.hpp
  ../TreeView.cpp
  ../TreeView.hpp
  ../TypeAndNameDialog.cpp
  ../TypeAndNameDialog.hpp
  ../UnknownTypeException.cpp
  ../UnknownTypeException.hpp
)

CF_ADD_UNITTEST( uTestClient )

ENDIF()
