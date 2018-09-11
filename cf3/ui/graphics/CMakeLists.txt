#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND coolfluid_ui_graphics_moc_classes
  Application.hpp             # needs to be added because base class emits signals
  BrowserDialog.hpp
  CentralPanel.hpp
  ModifiedOptionsDialog.hpp
  ConfirmCommitDialog.hpp
  FavoritesModel.hpp
  FileFilter.hpp              # needs to be added because base class emits signals
  FilteringModel.hpp          # needs to be added because base class emits signals
  GraphicalArray.hpp
  GraphicalArrayRestrictedList.hpp
  GraphicalBool.hpp
  GraphicalDouble.hpp
  GraphicalInt.hpp
  GraphicalRestrictedList.hpp
  GraphicalString.hpp
  GraphicalUri.hpp
  GraphicalUriArray.hpp
  GraphicalValue.hpp
  JournalBrowserDialog.hpp
  LoggingList.hpp
  MainWindow.hpp
  OptionLayout.hpp
  PythonCodeContainer.hpp
  PythonCodeEditor.hpp
  PythonConsole.hpp
  PythonSyntaxeHighlighter.hpp
  RemoteFileCopy.hpp
  SignalManager.hpp
  SignalInspectorDialog.hpp
  SignatureDialog.hpp
  SelectPathDialog.hpp
  TabBuilder.hpp
  TreeBrowser.hpp
  TreeView.hpp )

# Add the project files (including the moc files)
list( APPEND coolfluid_ui_graphics_files
  Application.cpp
  Application.hpp
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  BrowserDialog.cpp
  BrowserDialog.hpp
  CentralPanel.cpp
  CentralPanel.hpp
  ConfirmCommitDialog.cpp
  ConfirmCommitDialog.hpp
  FavoritesModel.cpp
  FavoritesModel.hpp
  FileFilter.cpp
  FileFilter.hpp
  FilesListItem.cpp
  FilesListItem.hpp
  FilteringModel.cpp
  FilteringModel.hpp
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
  LibGraphics.cpp
  LibGraphics.hpp
  LoggingList.cpp
  LoggingList.hpp
  MainWindow.cpp
  MainWindow.hpp
  ModifiedOptionsDialog.cpp
  ModifiedOptionsDialog.hpp
  OptionLayout.cpp
  OptionLayout.hpp
  PythonCodeContainer.cpp
  PythonCodeContainer.hpp
  PythonCodeEditor.cpp
  PythonCodeEditor.hpp
  PythonConsole.cpp
  PythonConsole.hpp
  PythonPreCompiler.cpp
  PythonPreCompiler.hpp
  PythonSyntaxeHighlighter.cpp
  PythonSyntaxeHighlighter.hpp
  RemoteFileCopy.cpp
  RemoteFileCopy.hpp
  SelectPathDialog.cpp
  SelectPathDialog.hpp
  SignalInspectorDialog.cpp
  SignalInspectorDialog.hpp
  SignalManager.cpp
  SignalManager.hpp
  SignatureDialog.cpp
  SignatureDialog.hpp
  TabBuilder.cpp
  TabBuilder.hpp
  TreeBrowser.cpp
  TreeBrowser.hpp
  TreeView.cpp
  TreeView.hpp
  TabBuilder.cpp
  TabBuilder.hpp
 )

if( CF3_ENABLE_GUI AND QT4_FOUND )
    QT4_ADD_RESOURCES(coolfluid_ui_graphics_RCC Resources/coolfluid-client.qrc)
endif()

coolfluid3_add_library( TARGET coolfluid_ui_graphics
                        SOURCES
							${coolfluid_ui_graphics_files}
						MOC
                            ${coolfluid_ui_graphics_moc_classes}
						RCC
                            ${coolfluid_ui_graphics_RCC}
                        INCLUDES
                            ${PYTHON_INCLUDE_DIRS}
                        LIBS
                            coolfluid_ui_core
                            ${Boost_PYTHON_LIBRARY}
                            ${PYTHON_LIBRARIES}
                            ${QT_LIBRARIES} coolfluid_qwt coolfluid_fparser
                        CONDITION
                            CF3_ENABLE_GUI AND QT4_FOUND )

