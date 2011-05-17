################################################################################
# Test Client core

if( QT4_FOUND AND CF_ENABLE_GUI )

set(utest-ui-graphics_moc_classes
  GraphicalArrayTest.hpp
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.hpp
  GraphicalStringTest.hpp
  GraphicalRestrictedListTest.hpp
  GraphicalUintTest.hpp
  GraphicalUriTest.hpp
  GraphicalValueTest.hpp
)

QT4_WRAP_CPP(utest-ui-graphics_moc_files ${utest-ui-graphics_moc_classes})

list( APPEND utest-ui-graphics_libs ${QT_LIBRARIES})
list( APPEND utest-ui-graphics_files
  utest-ui-graphics.cpp
  GraphicalArrayTest.cpp
  GraphicalArrayTest.hpp
  GraphicalBoolTest.cpp
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.cpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.cpp
  GraphicalIntTest.hpp
  GraphicalRestrictedListTest.cpp
  GraphicalRestrictedListTest.hpp
  GraphicalStringTest.cpp
  GraphicalStringTest.hpp
  GraphicalUintTest.cpp
  GraphicalUintTest.hpp
  GraphicalUriTest.cpp
  GraphicalUriTest.hpp
  GraphicalValueTest.cpp
  GraphicalValueTest.hpp
)

list( APPEND utest-ui-graphics_libs coolfluid_ui_graphics ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-ui-graphics )

else()

coolfluid_mark_not_orphan(
  utest-ui-graphics.cpp
  GraphicalArrayTest.cpp
  GraphicalArrayTest.hpp
  GraphicalBoolTest.cpp
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.cpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.cpp
  GraphicalIntTest.hpp
  GraphicalRestrictedListTest.cpp
  GraphicalRestrictedListTest.hpp
  GraphicalStringTest.cpp
  GraphicalStringTest.hpp
  GraphicalUintTest.cpp
  GraphicalUintTest.hpp
  GraphicalUriTest.cpp
  GraphicalUriTest.hpp
  GraphicalValueTest.cpp
  GraphicalValueTest.hpp
)

endif()
