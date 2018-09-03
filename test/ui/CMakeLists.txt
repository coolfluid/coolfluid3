  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})

#
# uTests for coolfluid_ui_core library
#

  coolfluid_add_test( UTEST      utest-ui-core-cnode
                      CPP        utest-ui-core-cnode.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-commit-details
                      CPP        utest-ui-core-commit-details.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-nbrowser
                      CPP        utest-ui-core-nbrowser.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-nlink
                      CPP        utest-ui-core-nlink.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-nroot
                      CPP        utest-ui-core-nroot.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-ntree
                      CPP        utest-ui-core-ntree.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-property-model
                      CPP        utest-ui-core-property-model.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)

  coolfluid_add_test( UTEST      utest-ui-core-tree-node
                      CPP        utest-ui-core-tree-node.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  coolfluid_ui_core_builds)


  # add_custom_command( TARGET utest-ui-core-cnode
  #                     POST_BUILD
  #                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/ )

#
# uTests for coolfluid_ui_graphics library
#

  coolfluid_add_test( UTEST      utest-ui-graphics-restricted-list
                      CPP        utest-ui-graphics-restricted-list.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-array
                      CPP        utest-ui-graphics-array.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-bool
                      CPP        utest-ui-graphics-bool.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-double
                      CPP        utest-ui-graphics-double.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-int
                      CPP        utest-ui-graphics-int.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-string
                      CPP        utest-ui-graphics-string.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-uint
                      CPP        utest-ui-graphics-uint.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-uri
                      CPP        utest-ui-graphics-uri.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

  coolfluid_add_test( UTEST      utest-ui-graphics-value
                      CPP        utest-ui-graphics-value.cpp Application.hpp
                      LIBS       coolfluid_ui_graphics
                      CONDITION  coolfluid_ui_graphics_builds )

#
# uTests for coolfluid_ui_network library
#

  coolfluid_add_test( UTEST      utest-ui-network-connection
                      CPP        utest-ui-network-connection.cpp
                      LIBS       coolfluid_ui_network  ${PTHREAD_LIBRARIES}
                      CONDITION  coolfluid_ui_network_builds )
