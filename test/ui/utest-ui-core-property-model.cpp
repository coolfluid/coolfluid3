// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui PropertyModel class"

#include "common/OptionURI.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "ui/core/NTree.hpp"
#include "ui/core/PropertyModel.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"

#include "test/ui/MyNode.hpp"
#include "ui/core/PropertyModel.hpp"

#include "test/ui/CoreApplication.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::core;
using namespace cf3::ui::CoreTest;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiCorePropertyModelSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();

  TreeThread& tree = ThreadManager::instance().tree();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  boost::shared_ptr< MyNode > node(new MyNode("MyAdditionalNode"));

  PropertyList& props = node->properties();
  OptionList& opts = node->options();

  // we clear existing properties and add new ones to ensure the tests will pass
  // even if properties of Component or CNode classes are modified.
  props.store.clear();
  opts.store.clear();

  opts.add("AnUriOption", URI("cpath:/"));
  props.add("Euler", Real(2.71));
  opts.add("MyString", std::string("Hello, World!"));
  props.add("Pi", Real(3.14159));
  opts.add("SomeBool", true);
  opts.add("SomeInt", int(-2168454));
  props.add("TheAnswer", Uint(42));

  BOOST_CHECK_EQUAL( props.store.size(), size_t(3));
  BOOST_CHECK_EQUAL( opts.store.size(), size_t(4));

  // add the node to the tree
  tree.root()->add_node( node );

  // set the node as the current index
  NTree::global()->set_current_index( NTree::global()->index_from_path( node->uri() ) );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data )
{
  PropertyModel model;
  int role = Qt::DisplayRole;

  // 1. give an invalid index
  BOOST_CHECK( !model.data(QModelIndex(), Qt::DisplayRole).isValid() );

  // 2. give a wrong role
  BOOST_CHECK( !model.data( model.index(0, 0), Qt::DecorationRole).isValid() );

  // 3. everything is OK
  BOOST_CHECK_EQUAL( model.data( model.index(0, 0), role ).toString().toStdString(), std::string("AnUriOption") );
  BOOST_CHECK_EQUAL( model.data( model.index(0, 1), role ).toString().toStdString(), std::string("cpath:/") );

  BOOST_CHECK_EQUAL( model.data( model.index(1, 0), role ).toString().toStdString(), std::string("Euler") );
  BOOST_CHECK_EQUAL( model.data( model.index(1, 1), role ).toString().toStdString(), std::string("2.71") );

  BOOST_CHECK_EQUAL( model.data( model.index(2, 0), role ).toString().toStdString(), std::string("MyString") );
  BOOST_CHECK_EQUAL( model.data( model.index(2, 1), role ).toString().toStdString(), std::string("Hello, World!") );

  BOOST_CHECK_EQUAL( model.data( model.index(3, 0), role ).toString().toStdString(), std::string("Pi") );
  BOOST_CHECK_EQUAL( model.data( model.index(3, 1), role ).toString().toStdString(), std::string("3.14159") );

  BOOST_CHECK_EQUAL( model.data( model.index(4, 0), role ).toString().toStdString(), std::string("SomeBool") );
  BOOST_CHECK_EQUAL( model.data( model.index(4, 1), role ).toString().toStdString(), std::string("true") );

  BOOST_CHECK_EQUAL( model.data( model.index(5, 0), role ).toString().toStdString(), std::string("SomeInt") );
  BOOST_CHECK_EQUAL( model.data( model.index(5, 1), role ).toString().toStdString(), std::string("-2168454") );

  BOOST_CHECK_EQUAL( model.data( model.index(6, 0), role ).toString().toStdString(), std::string("TheAnswer") );
  BOOST_CHECK_EQUAL( model.data( model.index(6, 1), role ).toString().toStdString(), std::string("42") );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( index )
{
  PropertyModel model;

  // we first check everything is OK since we will consider later that
  // index(0,0) returns a correct index

  // 1. everything is ok
  BOOST_CHECK( model.index(0, 0).isValid() );
  BOOST_CHECK( model.index(0, 1).isValid() );

  BOOST_CHECK( model.index(1, 0).isValid() );
  BOOST_CHECK( model.index(1, 1).isValid() );

  BOOST_CHECK( model.index(2, 0).isValid() );
  BOOST_CHECK( model.index(2, 1).isValid() );

  BOOST_CHECK( model.index(3, 0).isValid() );
  BOOST_CHECK( model.index(3, 1).isValid() );

  BOOST_CHECK( model.index(4, 0).isValid() );
  BOOST_CHECK( model.index(4, 1).isValid() );

  BOOST_CHECK( model.index(5, 0).isValid() );
  BOOST_CHECK( model.index(5, 1).isValid() );

  BOOST_CHECK( model.index(6, 0).isValid() );
  BOOST_CHECK( model.index(6, 1).isValid() );

  // 2. invalid row/column
  BOOST_CHECK( !model.index(7, 0).isValid() );
  BOOST_CHECK( !model.index(0, 2).isValid() );

  // 3. valid parent
  BOOST_CHECK( !model.index(0, 0, model.index(0, 0) ).isValid() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( row_count )
{
  PropertyModel model;

  // 1. invalid parent (should return the number of properties)
  BOOST_CHECK_EQUAL( model.rowCount( QModelIndex() ), 7);

  // 2. valid parent (should return 0)
  BOOST_CHECK_EQUAL( model.rowCount( model.index(0, 0) ), 0);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( header_data )
{
  PropertyModel model;

  BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().toStdString(), std::string("Name"));
  BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().toStdString(), std::string("Value"));

  BOOST_CHECK(!model.headerData(0, Qt::Vertical).isValid());
  BOOST_CHECK(!model.headerData(0, Qt::Horizontal, Qt::DecorationRole).isValid());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

