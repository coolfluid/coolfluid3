
coolfluid_add_test( UTEST      utest-ui-core
                    CPP        utest-ui-core.cpp
                               ExceptionThrowHandler.hpp
                               CNodeTest.hpp CNodeTest.cpp
                               CommitDetailsTest.cpp CommitDetailsTest.hpp
                               CommonFunctions.cpp CommonFunctions.hpp
                               MyNode.cpp MyNode.hpp
                               NBrowserTest.cpp NBrowserTest.hpp
                               NLinkTest.cpp NLinkTest.hpp
                               NRootTest.cpp NRootTest.hpp
                               NTreeTest.hpp NTreeTest.cpp
                               PropertyModelTest.cpp PropertyModelTest.hpp
                               TreeHandler.cpp TreeHandler.hpp
                               TreeNodeTest.cpp TreeNodeTest.hpp
                    MOC        CommonFunctions.hpp
                               CommitDetailsTest.hpp
                               CNodeTest.hpp
                               NBrowserTest.hpp
                               NLinkTest.hpp
                               NRootTest.hpp
                               NTreeTest.hpp
                               PropertyModelTest.hpp
                               TreeNodeTest.hpp
                    LIBS       coolfluid_ui_core ${QT_LIBRARIES}
                    CONDITION  CF_ENABLE_GUI )

add_custom_command( TARGET utest-ui-core
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   )
