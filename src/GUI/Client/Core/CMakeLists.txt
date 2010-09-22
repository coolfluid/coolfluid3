#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND ClientCore_moc_classes
CNode.hpp
ClientNetworkComm.hpp
NArray.hpp
NCore.hpp
NElements.hpp
NGroup.hpp
NLink.hpp
NLog.hpp
NMesh.hpp
NMeshReader.hpp
NMethod.hpp
NRegion.hpp
NTable.hpp
NTree.hpp
PropertyModel.hpp
StatusModel.hpp
TreeNode.hpp
)

# Create the moc files
QT4_WRAP_CPP(ClientCore_moc_files ${ClientCore_moc_classes})

# Add the project files (including the moc files)
list( APPEND ClientCore_files
CNode.cpp
CNode.hpp
ClientNetworkComm.cpp
ClientNetworkComm.hpp
ClientRoot.cpp
ClientRoot.hpp
CommitDetails.cpp
CommitDetails.hpp
CommitDetailsItem.cpp
CommitDetailsItem.hpp
NArray.cpp
NArray.hpp
NBrowser.cpp
NBrowser.hpp
NCore.cpp
NCore.hpp
NElements.cpp
NElements.hpp
NGroup.cpp
NGroup.hpp
NLink.cpp
NLink.hpp
NLog.cpp
NLog.hpp
NMesh.cpp
NMesh.hpp
NMeshReader.cpp
NMeshReader.hpp
NMethod.cpp
NMethod.hpp
NRegion.cpp
NRegion.hpp
NRoot.cpp
NRoot.hpp
NTable.cpp
NTable.hpp
NTree.cpp
NTree.hpp
OptionType.cpp
OptionType.hpp
PropertyModel.cpp
PropertyModel.hpp
StatusModel.cpp
StatusModel.hpp
TSshInformation.hpp
TreeNode.cpp
TreeNode.hpp
UnknownTypeException.cpp
UnknownTypeException.hpp
)

# Use Qt Libraries
list( APPEND ClientCore_libs Network ${QT_LIBRARIES})

coolfluid_add_library(ClientCore)
