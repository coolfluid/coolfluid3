#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND coolfluid_client_ui_moc_classes
  BodePlot.hpp
  CentralPanel.hpp
  ColorSelector.hpp
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.hpp
  FilteringModel.hpp # needs to be added because base class emits signals
  Graph.hpp
  GraphicalArray.hpp
  GraphicalArrayRestrictedList.hpp
  GraphicalBool.hpp
  GraphicalDouble.hpp
  GraphicalInt.hpp
  GraphOption.hpp
  GraphicalRestrictedList.hpp
  GraphicalString.hpp
  GraphicalUri.hpp
  GraphicalUriArray.hpp
  GraphicalValue.hpp
  JournalBrowserDialog.hpp
  LoggingList.hpp
  MainWindow.hpp
  NRemoteBrowser.hpp
  NRemoteSave.hpp
  OptionLayout.hpp
  SelectFileDialog.hpp
  SignalManager.hpp
  SignalInspectorDialog.hpp
  SignatureDialog.hpp
  StatusPanel.hpp
  SelectPathDialog.hpp
  TreeBrowser.hpp
  TreeView.hpp
  TypeAndNameDialog.hpp)

# Create the moc files
QT4_WRAP_CPP(coolfluid_client_ui_moc_files ${coolfluid_client_ui_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid_client_ui_files
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  BodePlot.cpp
  BodePlot.hpp
  CentralPanel.cpp
  CentralPanel.hpp
  ColorSelector.cpp
  ColorSelector.hpp
  CommitDetailsDialog.cpp
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.cpp
  ConfirmCommitDialog.hpp
  FilesListItem.cpp
  FilesListItem.hpp
  FilteringModel.cpp
  FilteringModel.hpp
  Graph.hpp
  Graph.cpp
  GraphicalArray.hpp
  GraphicalArray.cpp
  GraphicalArrayRestrictedList.hpp
  GraphicalArrayRestrictedList.cpp
  GraphicalBool.hpp
  GraphicalBool.cpp
  GraphicalDouble.hpp
  GraphicalDouble.cpp
  GraphicalInt.hpp
  GraphicalInt.cpp
  GraphOption.cpp
  GraphOption.hpp
  GraphicalRestrictedList.cpp
  GraphicalRestrictedList.hpp
  GraphicalString.hpp
  GraphicalString.cpp
  GraphicalUri.cpp
  GraphicalUri.hpp
  GraphicalUriArray.cpp
  GraphicalUriArray.hpp
  GraphicalValue.cpp
  GraphicalValue.hpp
  JournalBrowserDialog.cpp
  JournalBrowserDialog.hpp
  LibClientUI.cpp
  LibClientUI.hpp
  LoggingList.cpp
  LoggingList.hpp
  MainWindow.cpp
  MainWindow.hpp
  MenuActionInfo.cpp
  MenuActionInfo.hpp
  OptionLayout.cpp
  OptionLayout.hpp
  NRemoteBrowser.cpp
  NRemoteBrowser.hpp
  NRemoteOpen.cpp
  NRemoteOpen.hpp
  NRemoteSave.cpp
  NRemoteSave.hpp
  PixMaps.hpp
  SelectFileDialog.cpp
  SelectFileDialog.hpp
  SelectPathDialog.cpp
  SelectPathDialog.hpp
  SignalInspectorDialog.cpp
  SignalInspectorDialog.hpp
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
  TypeAndNameDialog.hpp )

list( APPEND coolfluid_client_ui_cflibs coolfluid_client_core )

# Use Qt Libraries
list( APPEND coolfluid_client_ui_libs ${QT_LIBRARIES} coolfluid_qwt)

QT4_ADD_RESOURCES(coolfluid_client_ui_RCC Resources/coolfluid-client.qrc)

coolfluid_add_library(coolfluid_client_ui)
