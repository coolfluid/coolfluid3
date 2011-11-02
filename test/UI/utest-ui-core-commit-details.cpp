
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the UI CommitDetails class"

#include "UI/Core/CommitDetails.hpp"
#include "UI/Core/CommitDetailsItem.hpp"

#include "test/UI/CoreApplication.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::ui::core;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( UICoreCommitDetailsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  application();

  AssertionManager::instance().AssertionDumps = false;
  AssertionManager::instance().AssertionThrows = true;
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionOutputs = false;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data )
{
  CommitDetails cd;
  Qt::ItemDataRole role = Qt::DisplayRole;

  cd.set_option("Option1", "OldVal1", "NewVal1");
  cd.set_option("Option2", "OldVal2", "");
  cd.set_option("Option3", "", "NewVal3");

  // 1. invalid index
  BOOST_CHECK( !cd.data(QModelIndex(), Qt::DecorationRole).isValid() );

  // 2. everything is ok (row 0)
  BOOST_CHECK_EQUAL( cd.data( cd.index(0, 0), role).toString().toStdString(), std::string("Option1") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(0, 1), role).toString().toStdString(), std::string("\"OldVal1\"") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(0, 2), role).toString().toStdString(), std::string("\"NewVal1\"") );

  // 3. new value is empty (row 1)
  BOOST_CHECK_EQUAL( cd.data( cd.index(1, 0), role).toString().toStdString(), std::string("Option2") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(1, 1), role).toString().toStdString(), std::string("\"OldVal2\"") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(1, 2), role).toString().toStdString(), std::string("--") );

  // 4. old value is empty (row 2)
  BOOST_CHECK_EQUAL( cd.data( cd.index(2, 0), role).toString().toStdString(), std::string("Option3") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(2, 1), role).toString().toStdString(), std::string("--") );
  BOOST_CHECK_EQUAL( cd.data( cd.index(2, 2), role).toString().toStdString(), std::string("\"NewVal3\"") );

  // 5. wrong rolw
  BOOST_CHECK( !cd.data( cd.index(0, 0), Qt::DecorationRole).isValid() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( header_data )
{
  CommitDetails cd;
  Qt::ItemDataRole role = Qt::DisplayRole;

  // 1. check column headers
  BOOST_CHECK_EQUAL( cd.headerData(0, Qt::Horizontal, role).toString().toStdString(), std::string("Name") );
  BOOST_CHECK_EQUAL( cd.headerData(1, Qt::Horizontal, role).toString().toStdString(), std::string("Old Value") );
  BOOST_CHECK_EQUAL( cd.headerData(2, Qt::Horizontal, role).toString().toStdString(), std::string("New Value") );

  // 2. check row headers
  BOOST_CHECK_EQUAL( cd.headerData(0, Qt::Vertical, role).toString().toStdString(), std::string("Option #1") );
  BOOST_CHECK_EQUAL( cd.headerData(14, Qt::Vertical, role).toString().toStdString(), std::string("Option #15") );
  BOOST_CHECK_EQUAL( cd.headerData(8790, Qt::Vertical, role).toString().toStdString(), std::string("Option #8791") );

  // 3. wrong column number
  BOOST_CHECK( !cd.headerData(3, Qt::Horizontal, role).isValid() );

  // 4. wrong role
  BOOST_CHECK( !cd.headerData(1, Qt::Horizontal, Qt::DecorationRole).isValid() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( index )
{
  CommitDetails cd;

  BOOST_CHECK( !cd.index(0, 0).isValid() );

  cd.set_option("Option1", "OldVal", "NewVal");

  QModelIndex index = cd.index(0, 0);
  CommitDetailsItem * item = static_cast<CommitDetailsItem*>(index.internalPointer());
  BOOST_CHECK( is_not_null(item) );

  BOOST_CHECK_EQUAL( item->option_name().toStdString(), std::string("Option1") );
  BOOST_CHECK_EQUAL( item->old_value().toStdString(), std::string("OldVal") );
  BOOST_CHECK_EQUAL( item->current_value().toStdString(), std::string("NewVal") );

  BOOST_CHECK( !cd.index(0, 0, index).isValid() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( set_option )
{
  CommitDetails cd;

  BOOST_CHECK( !cd.has_options() );
  cd.set_option("Option1", "OldVal", "NewVal");
  BOOST_CHECK( cd.has_options() );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( clear )
{
  CommitDetails cd(nullptr, "//Path/To/Node");

  BOOST_CHECK( !cd.has_options() );

  BOOST_CHECK_EQUAL( cd.node_path().toStdString(), std::string("//Path/To/Node") );
  cd.set_option("Option1", "OldVal", "NewVal");

  BOOST_CHECK( cd.has_options() );

  cd.clear();

  BOOST_CHECK( !cd.has_options() );
  BOOST_CHECK( cd.node_path().isEmpty() );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
