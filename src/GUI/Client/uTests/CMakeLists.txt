################################################################################
# Test Client

IF( QT_QTTEST_FOUND )

SET(uTestClient_moc_classes

  NTreeTest.hpp
  CNodeTest.hpp

  # include files to moc from the client

#  ../AddLinkDialog.hpp
  ../ClientCore.hpp
  ../ClientNetworkComm.hpp
  ../CNode.hpp
#  ../CommitDetailsDialog.hpp
#  ../ConfirmCommitDialog.hpp
#  ../ConnectionDialog.hpp
#  ../FilesPanel.hpp
#  ../LoggingList.hpp
#  ../MainWindow.hpp
  ../NGroup.hpp
  ../NLink.hpp
  ../NLog.hpp
  ../NMesh.hpp
  ../NMethod.hpp
  ../NTree.hpp
  ../TreeNode.hpp
#  ../OptionPanel.hpp
#  ../RemoteFSBrowser.hpp
#  ../RemoteSaveFile.hpp
#  ../SelectFileDialog.hpp
#  ../StatusModel.hpp
#  ../StatusPanel.hpp
#  ../TreeView.hpp
#  ../TypeAndNameDialog.hpp
)

QT4_WRAP_CPP(uTestClient_moc_files ${uTestClient_moc_classes})

CONFIGURE_FILE ( tree.xml      ${CMAKE_CURRENT_BINARY_DIR}/tree.xml       @ONLY )

LIST( APPEND uTestClient_libs Common Network ${QT_LIBRARIES})
LIST( APPEND uTestClient_files
  uTestClient.cpp
  ExceptionThrowHandler.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  CNodeTest.hpp
  CNodeTest.cpp
  MyNode.cpp
  MyNode.hpp

  # include files from the client

#  ../AboutCFDialog.cpp
#  ../AboutCFDialog.hpp
#  ../AddLinkDialog.cpp
#  ../AddLinkDialog.hpp
  ../ClientCore.cpp
  ../ClientCore.hpp
  ../ClientNetworkComm.cpp
  ../ClientNetworkComm.hpp
  ../ClientRoot.cpp
  ../ClientRoot.hpp
  ../CNode.cpp
  ../CNode.hpp
#  ../CommitDetails.cpp
#  ../CommitDetails.hpp
#  ../CommitDetailsDialog.cpp
#  ../CommitDetailsDialog.hpp
#  ../CommitDetailsItem.cpp
#  ../CommitDetailsItem.hpp
#  ../ConfirmCommitDialog.cpp
#  ../ConfirmCommitDialog.hpp
#  ../ConnectionDialog.cpp
#  ../ConnectionDialog.hpp
#  ../FilesListItem.cpp
#  ../FilesListItem.hpp
#  ../FilesPanel.cpp
#  ../FilesPanel.hpp
#  ../GraphicalOption.cpp
#  ../GraphicalOption.hpp
#  ../HostListPanel.cpp
#  ../HostListPanel.hpp
  ../InvalidValue.cpp
  ../InvalidValue.hpp
#  ../LibrariesPanel.cpp
#  ../LibrariesPanel.hpp
#  ../LoggingList.cpp
#  ../LoggingList.hpp
#  ../main.cpp
#  ../MainWindow.cpp
#  ../MainWindow.hpp
#  ../MenuActionInfo.cpp
#  ../MenuActionInfo.hpp
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
#  ../OptionPanel.cpp
#  ../OptionPanel.hpp
  ../OptionType.cpp
  ../OptionType.hpp
#  ../RemoteFSBrowser.cpp
#  ../RemoteFSBrowser.hpp
#  ../RemoteOpenFile.cpp
#  ../RemoteOpenFile.hpp
#  ../RemoteSaveFile.cpp
#  ../RemoteSaveFile.hpp
#  ../SelectFileDialog.cpp
#  ../SelectFileDialog.hpp
#  ../StatusModel.cpp
#  ../StatusModel.hpp
#  ../StatusPanel.cpp
#  ../StatusPanel.hpp
  ../TObjectProperties.hpp
  ../TreeNode.cpp
  ../TreeNode.hpp
#  ../TreeView.cpp
#  ../TreeView.hpp
  ../TSshInformation.hpp
#  ../TypeAndNameDialog.cpp
#  ../TypeAndNameDialog.hpp
  ../UnknownTypeException.cpp
  ../UnknownTypeException.hpp
)

CF_ADD_UNITTEST( uTestClient )

ENDIF()
