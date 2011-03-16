################################################################################
# Test Client core

set(utest-client-core_moc_classes
  CommonFunctions.hpp
  CommitDetailsTest.hpp
  CNodeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  NRootTest.hpp
  NTreeTest.hpp
  PropertyModelTest.hpp
  TreeNodeTest.hpp
)

QT4_WRAP_CPP(utest-client-core_moc_files ${utest-client-core_moc_classes})

#configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND utest-client-core_cflibs coolfluid_network)
list( APPEND utest-client-core_libs ${QT_LIBRARIES})
list( APPEND utest-client-core_files
  utest-client-core.cpp
  ExceptionThrowHandler.hpp
  CNodeTest.hpp
  CNodeTest.cpp
  CommitDetailsTest.cpp
  CommitDetailsTest.hpp
  CommonFunctions.cpp
  CommonFunctions.hpp
  MyNode.cpp
  MyNode.hpp
  NBrowserTest.cpp
  NBrowserTest.hpp
  NLinkTest.cpp
  NLinkTest.hpp
  NRootTest.cpp
  NRootTest.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  PropertyModelTest.cpp
  PropertyModelTest.hpp
  TreeHandler.cpp
  TreeHandler.hpp
  TreeNodeTest.cpp
  TreeNodeTest.hpp
)

list( APPEND utest-client-core_libs coolfluid_client_core ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-client-core )

add_custom_command(TARGET utest-client-core
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
