
coolfluid_add_test( UTEST      utest-ui-graphics
                    CPP        utest-ui-graphics.cpp
                               GraphicalArrayTest.cpp GraphicalArrayTest.hpp
                               GraphicalBoolTest.cpp GraphicalBoolTest.hpp
                               GraphicalDoubleTest.cpp GraphicalDoubleTest.hpp
                               GraphicalIntTest.cpp GraphicalIntTest.hpp
                               GraphicalRestrictedListTest.cpp GraphicalRestrictedListTest.hpp
                               GraphicalStringTest.cpp GraphicalStringTest.hpp
                               GraphicalUintTest.cpp GraphicalUintTest.hpp
                               GraphicalUriTest.cpp GraphicalUriTest.hpp
                               GraphicalValueTest.cpp GraphicalValueTest.hpp
                    MOC        GraphicalArrayTest.hpp
                               GraphicalBoolTest.hpp
                               GraphicalDoubleTest.hpp
                               GraphicalIntTest.hpp
                               GraphicalStringTest.hpp
                               GraphicalRestrictedListTest.hpp
                               GraphicalUintTest.hpp
                               GraphicalUriTest.hpp
                               GraphicalValueTest.hpp
                    LIBS       coolfluid_ui_graphics ${QT_LIBRARIES}
                    CONDITION  CF3_ENABLE_Gui )
