#ifndef CF_GUI_Client_uTests_NTreeTest_hpp
#define CF_GUI_Client_uTests_NTreeTest_hpp

///////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/NRoot.hpp"

///////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

  /////////////////////////////////////////////////////////////////////////

  class NTreeTest : public QObject
  {
    Q_OBJECT

  private slots:

    void initTestCase();

    void test_constructor();

    void test_setRoot();

    void test_setCurrentIndex();

    void test_getNodeParams();

    void test_setAdvancedMode();

    void test_areFromSameNode();

    void test_haveSameData();

    void test_getNodeByPath();

    void test_getIndexByPath();

    void test_data();

    void test_index();

    void test_parent();

    void test_rowCount();

    void test_headerData();

    void test_setDebugModeEnabled();

    void test_list_tree();

  private:

    static CF::GUI::Client::NRoot::Ptr makeTreeFromFile();

  }; // class NTreeTest

  /////////////////////////////////////////////////////////////////////////

} // namespace ClientTest
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_NTreeTest_hpp
