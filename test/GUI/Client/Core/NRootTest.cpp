// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/Core/NGeneric.hpp"
#include "GUI/Client/Core/NRoot.hpp"

#include "test/GUI/Client/Core/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/Core/NRootTest.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientTest {

////////////////////////////////////////////////////////////////////////////////

void NRootTest::test_childFromRoot()
{
  NRoot::Ptr root( new NRoot("Root") );
  NGeneric::Ptr node1(new NGeneric("Node1", "MyFirstType"));
  NGeneric::Ptr node2(new NGeneric("Node2", "MySecondType"));
  CNode::Ptr node;

  // 1. root has no child
  GUI_CHECK_THROW( root->childFromRoot(0), FailedAssertion );
  GUI_CHECK_THROW( root->childFromRoot(1), FailedAssertion );

  root->addNode(node1);
  root->addNode(node2);

  // 2. get the first child (node1)
  GUI_CHECK_NO_THROW( node = root->childFromRoot(0) );
  QVERIFY( node->checkType(CNode::GENERIC_NODE) );
  QCOMPARE( QString(node->name().c_str()), QString("Node1") );
  QCOMPARE( node->getComponentType(), QString("MyFirstType") );

  // 3. get the second child (node2)
  GUI_CHECK_NO_THROW( node = root->childFromRoot(1) );
  QVERIFY( node->checkType(CNode::GENERIC_NODE) );
  QCOMPARE( QString(node->name().c_str()), QString("Node2") );
  QCOMPARE( node->getComponentType(), QString("MySecondType") );
}

///////////////////////////////////////////////////////////////////////////////

} // ClientTest
} // GUI
} // CF
