################################################################################
# Test Client

if( QT_QTTEST_FOUND )

set(uTestClient_moc_classes

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
  ../GraphicalBool.hpp
  ../GraphicalDouble.hpp
  ../GraphicalInt.hpp
  ../GraphicalOption.hpp
  ../GraphicalString.hpp
  ../GraphicalUrl.hpp
  ../GraphicalUrlArray.hpp
  ../GraphicalValue.hpp
  ../LoggingList.hpp
  ../MainWindow.hpp
  ../NArray.hpp
  ../NCore.hpp
  ../NElements.hpp
  ../NGroup.hpp
  ../NLink.hpp
  ../NLog.hpp
  ../NMesh.hpp
  ../NMeshReader.hpp
  ../NMethod.hpp
  ../NRegion.hpp
  ../NTable.hpp
  ../NTree.hpp
  ../OptionPanel.hpp
  ../NRemoteBrowser.hpp
  ../NRemoteSave.hpp
  ../PropertyModel.hpp
  ../SelectFileDialog.hpp
  ../SelectPathDialog.hpp
  ../StatusModel.hpp
  ../StatusPanel.hpp
  ../TreeBrowser.hpp
  ../TreeNode.hpp
  ../TreeView.hpp
  ../TypeAndNameDialog.hpp
)

QT4_WRAP_CPP(uTestClient_moc_files ${uTestClient_moc_classes})

configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND uTestClient_cflibs Network)
list( APPEND uTestClient_libs ${QT_LIBRARIES})
list( APPEND uTestClient_files
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
  ../GraphicalBool.hpp
  ../GraphicalBool.cpp
  ../GraphicalDouble.hpp
  ../GraphicalDouble.cpp
  ../GraphicalInt.hpp
  ../GraphicalInt.cpp
  ../GraphicalOption.cpp
  ../GraphicalOption.hpp
  ../GraphicalString.hpp
  ../GraphicalString.cpp
  ../GraphicalUrl.cpp
  ../GraphicalUrl.hpp
  ../GraphicalUrlArray.cpp
  ../GraphicalUrlArray.hpp
  ../GraphicalValue.cpp
  ../GraphicalValue.hpp
  ../LoggingList.cpp
  ../LoggingList.hpp
  ../MainWindow.cpp
  ../MainWindow.hpp
  ../MenuActionInfo.cpp
  ../MenuActionInfo.hpp
  ../NArray.cpp
  ../NArray.hpp
  ../NBrowser.cpp
  ../NBrowser.hpp
  ../NCore.cpp
  ../NCore.hpp
  ../NElements.cpp
  ../NElements.hpp
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
  ../NRegion.cpp
  ../NRegion.hpp
  ../NRoot.cpp
  ../NRoot.hpp
  ../NTable.cpp
  ../NTable.hpp
  ../NTree.cpp
  ../NTree.hpp
  ../OptionPanel.cpp
  ../OptionPanel.hpp
  ../OptionType.cpp
  ../OptionType.hpp
  ../NRemoteBrowser.cpp
  ../NRemoteBrowser.hpp
  ../NRemoteOpen.cpp
  ../NRemoteOpen.hpp
  ../NRemoteSave.cpp
  ../NRemoteSave.hpp
  ../PropertyModel.cpp
  ../PropertyModel.hpp
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
  ../TreeBrowser.cpp
  ../TreeBrowser.hpp
  ../TreeView.cpp
  ../TreeView.hpp
  ../TypeAndNameDialog.cpp
  ../TypeAndNameDialog.hpp
  ../UnknownTypeException.cpp
  ../UnknownTypeException.hpp
)

coolfluid_add_unittest( uTestClient )

endif()
