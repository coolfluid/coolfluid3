#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND ClientUI_moc_classes
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.hpp
  ConnectionDialog.hpp
  FilteringModel.hpp # needs to be added because base class emits signals
  GraphicalBool.hpp
  GraphicalDouble.hpp
  GraphicalInt.hpp
  GraphicalOption.hpp
  GraphicalString.hpp
  GraphicalUrl.hpp
  GraphicalUrlArray.hpp
  GraphicalValue.hpp
  LoggingList.hpp
  MainWindow.hpp
  NRemoteBrowser.hpp
  NRemoteSave.hpp
  OptionPanel.hpp
  SelectFileDialog.hpp
  SignalManager.hpp
  SignatureDialog.hpp
  StatusPanel.hpp
  SelectPathDialog.hpp
  TreeBrowser.hpp
  TreeView.hpp
  TypeAndNameDialog.hpp)

# Create the moc files
QT4_WRAP_CPP(ClientUI_moc_files ${ClientUI_moc_classes})

# Add the project files (including the moc files)
list( APPEND ClientUI_files
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  CommitDetailsDialog.cpp
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.cpp
  ConfirmCommitDialog.hpp
  ConnectionDialog.cpp
  ConnectionDialog.hpp
  FilesListItem.cpp
  FilesListItem.hpp
  FilteringModel.cpp
  FilteringModel.hpp
  GraphicalBool.hpp
  GraphicalBool.cpp
  GraphicalDouble.hpp
  GraphicalDouble.cpp
  GraphicalInt.hpp
  GraphicalInt.cpp
  GraphicalOption.cpp
  GraphicalOption.hpp
  GraphicalString.hpp
  GraphicalString.cpp
  GraphicalUrl.cpp
  GraphicalUrl.hpp
  GraphicalUrlArray.cpp
  GraphicalUrlArray.hpp
  GraphicalValue.cpp
  GraphicalValue.hpp
  LibClientUI.cpp
  LibClientUI.hpp
  LoggingList.cpp
  LoggingList.hpp
  MainWindow.cpp
  MainWindow.hpp
  MenuActionInfo.cpp
  MenuActionInfo.hpp
  OptionPanel.cpp
  OptionPanel.hpp
  NRemoteBrowser.cpp
  NRemoteBrowser.hpp
  NRemoteOpen.cpp
  NRemoteOpen.hpp
  NRemoteSave.cpp
  NRemoteSave.hpp
  SelectFileDialog.cpp
  SelectFileDialog.hpp
  SelectPathDialog.cpp
  SelectPathDialog.hpp
  SignalManager.cpp
  SignalManager.hpp
  SignatureDialog.cpp
  SignatureDialog.hpp
  StatusPanel.cpp
  StatusPanel.hpp
  TreeBrowser.cpp
  TreeBrowser.hpp
  TreeView.cpp
  TreeView.hpp
  TypeAndNameDialog.cpp
  TypeAndNameDialog.hpp)

# Use Qt Libraries
list( APPEND ClientUI_libs ClientCore ${QT_LIBRARIES})

QT4_ADD_RESOURCES(ClientUI_RCC Resources/coolfluid-client.qrc)

coolfluid_add_library(ClientUI)
