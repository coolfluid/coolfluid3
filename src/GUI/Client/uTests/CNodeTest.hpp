#ifndef CF_GUI_Client_uTests_CNodeTest_hpp
#define CF_GUI_Client_uTests_CNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

  //////////////////////////////////////////////////////////////////////////

  class CNodeTest : public QObject
  {
    Q_OBJECT

  private slots:

      void test_getComponentType();

      void test_isClientComponent();

      void test_getType();

      void test_checkType();

      void test_setOptions();

      void test_getOptions();

      void test_createFromXml();

      void test_addNode();

  }; // class CNodeTest

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientTest
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_CNodeTest_hpp
