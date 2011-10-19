// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>

#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"

#include "UI/Core/NTree.hpp"
#include "UI/Core/PropertyModel.hpp"
#include "UI/Core/ThreadManager.hpp"
#include "UI/Core/TreeThread.hpp"

#include "test/UI/Core/MyNode.hpp"
#include "test/UI/Core/PropertyModelTest.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

///////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::initTestCase()
{
  MyNode::Ptr node(new MyNode("MyAdditionalNode"));

  PropertyList& props = node->properties();
  OptionList& opts = node->options();

  // we clear existing properties and add new ones to ensure the tests will pass
  // even if properties of Component or CNode classes are modified.
  props.store.clear();
  opts.store.clear();

  opts.add_option< OptionURI >("AnUriOption", URI("cpath://Root"));
  props.add_property("Euler", Real(2.71));
  opts.add_option< OptionT<std::string> >("MyString", std::string("Hello, World!"));
  props.add_property("Pi", Real(3.14159));
  opts.add_option< OptionT<bool> >("SomeBool", true);
  opts.add_option< OptionT<int> >("SomeInt", int(-2168454));
  props.add_property("TheAnswer", Uint(42));

  QCOMPARE( props.store.size(), size_t(3));
  QCOMPARE( opts.store.size(), size_t(4));

  // add the node to the tree
  ThreadManager::instance().tree().root()->addNode( node );

  // set the node as the current index
  NTree::globalTree()->setCurrentIndex( NTree::globalTree()->indexFromPath( node->uri() ) );
}

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::test_dataFunction()
{
  PropertyModel model;
  int role = Qt::DisplayRole;

  // 1. give an invalid index
  QVERIFY( !model.data(QModelIndex(), Qt::DisplayRole).isValid() );

  // 2. give a wrong role
  QVERIFY( !model.data( model.index(0, 0), Qt::DecorationRole).isValid() );

  // 3. everything is OK
  QCOMPARE( model.data( model.index(0, 0), role ).toString(), QString("AnUriOption") );
  QCOMPARE( model.data( model.index(0, 1), role ).toString(), QString("cpath://Root") );

  QCOMPARE( model.data( model.index(1, 0), role ).toString(), QString("Euler") );
  QCOMPARE( model.data( model.index(1, 1), role ).toString(), QString("2.71") );

  QCOMPARE( model.data( model.index(2, 0), role ).toString(), QString("MyString") );
  QCOMPARE( model.data( model.index(2, 1), role ).toString(), QString("Hello, World!") );

  QCOMPARE( model.data( model.index(3, 0), role ).toString(), QString("Pi") );
  QCOMPARE( model.data( model.index(3, 1), role ).toString(), QString("3.14159") );

  QCOMPARE( model.data( model.index(4, 0), role ).toString(), QString("SomeBool") );
  QCOMPARE( model.data( model.index(4, 1), role ).toString(), QString("true") );

  QCOMPARE( model.data( model.index(5, 0), role ).toString(), QString("SomeInt") );
  QCOMPARE( model.data( model.index(5, 1), role ).toString(), QString("-2168454") );

  QCOMPARE( model.data( model.index(6, 0), role ).toString(), QString("TheAnswer") );
  QCOMPARE( model.data( model.index(6, 1), role ).toString(), QString("42") );
}

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::test_index()
{
  PropertyModel model;

  // we first check everything is OK since we will consider later that
  // index(0,0) returns a correct index

  // 1. everything is ok
  QVERIFY( model.index(0, 0).isValid() );
  QVERIFY( model.index(0, 1).isValid() );

  QVERIFY( model.index(1, 0).isValid() );
  QVERIFY( model.index(1, 1).isValid() );

  QVERIFY( model.index(2, 0).isValid() );
  QVERIFY( model.index(2, 1).isValid() );

  QVERIFY( model.index(3, 0).isValid() );
  QVERIFY( model.index(3, 1).isValid() );

  QVERIFY( model.index(4, 0).isValid() );
  QVERIFY( model.index(4, 1).isValid() );

  QVERIFY( model.index(5, 0).isValid() );
  QVERIFY( model.index(5, 1).isValid() );

  QVERIFY( model.index(6, 0).isValid() );
  QVERIFY( model.index(6, 1).isValid() );

  // 2. invalid row/column
  QVERIFY( !model.index(7, 0).isValid() );
  QVERIFY( !model.index(0, 2).isValid() );

  // 3. valid parent
  QVERIFY( !model.index(0, 0, model.index(0, 0) ).isValid() );
}

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::test_rowCount()
{
  PropertyModel model;

  // 1. invalid parent (should return the number of properties)
  QCOMPARE( model.rowCount( QModelIndex() ), 7);

  // 2. valid parent (should return 0)
  QCOMPARE( model.rowCount( model.index(0, 0) ), 0);
}

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::test_headerData()
{
  PropertyModel model;

  QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), QString("Name"));
  QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), QString("Value"));

  QVERIFY(!model.headerData(0, Qt::Vertical).isValid());
  QVERIFY(!model.headerData(0, Qt::Horizontal, Qt::DecorationRole).isValid());
}

/////////////////////////////////////////////////////////////////////////

void PropertyModelTest::cleanupTestCase()
{
  ThreadManager::instance().tree().root()->removeNode("MyAdditionalNode");
}

/////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3
