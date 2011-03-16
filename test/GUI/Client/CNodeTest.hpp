// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_uTests_CNodeTest_hpp
#define CF_GUI_Client_uTests_CNodeTest_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDebug>
#include <QObject>

#include "Common/StringConversion.hpp"

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

  void test_setProperties();

  void test_setSignals();

  void test_modifyOptions();

  void test_listOptions();

  void test_listProperties();

  void test_createFromXml();

  void test_addNode();

  void test_removeNode();

  void test_listChildPaths();

  void test_makeOption();

  void test_makeOptionTypes();

  void test_makeOptionUriSchemes();

  void test_makeOptionRestrictedLists();

  void test_makeOptionArrayTypes();

  void test_makeOptionArrayRestrictedLists();

  // signals

  void test_configure_reply();

  void test_list_content_reply();

  void test_signal_signature_reply();

private:

  template<typename TYPE>
  bool compareVectors(const std::vector<TYPE> & left, const std::vector<TYPE> & right)
  {
    bool equal = left.size() == right.size();

    if(!equal)
      qDebug() << "Sizes are different. Left has" << left.size() <<
                  "elements whereas right has"<< right.size() << "elements.";

    for(unsigned int i = 0 ; i < left.size() && equal ; ++i)
    {
      equal = left[i] == right[i];

      if(!equal)
        qDebug() <<  "Item" << i << ": [" << Common::to_str(left[i]).c_str() <<
                     "] is different from [" << Common::to_str(right[i]).c_str() << "].";
    }

    return equal;
  }


}; // class CNodeTest

//////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_uTests_CNodeTest_hpp
