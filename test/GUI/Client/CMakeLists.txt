################################################################################
# Test Client

if( QT_QTTEST_FOUND AND CF_ENABLE_GUI)

set(utest-client_moc_classes
  CommonFunctions.hpp
  NTreeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  CNodeTest.hpp
  CommitDetailsTest.hpp
)

QT4_WRAP_CPP(utest-client_moc_files ${utest-client_moc_classes})

#configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND utest-client_cflibs coolfluid_network)
list( APPEND utest-client_libs ${QT_LIBRARIES})
list( APPEND utest-client_files
  utest-client.cpp
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
  NTreeTest.hpp
  NTreeTest.cpp
  TreeHandler.cpp
  TreeHandler.hpp
)

list( APPEND utest-client_libs coolfluid_client_core ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-client )

add_custom_command(TARGET utest-client
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

else()
    coolfluid_mark_not_orphan(
  utest-client.cpp
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
  NTreeTest.hpp
  NTreeTest.cpp
  TreeHandler.cpp
  TreeHandler.hpp
        )
endif()
