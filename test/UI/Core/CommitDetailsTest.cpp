// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QtTest>

#include "UI/Core/CommitDetailsItem.hpp"
#include "UI/Core/CommitDetails.hpp"

#include "test/UI/Core/CommitDetailsTest.hpp"

using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace CoreTest {

//////////////////////////////////////////////////////////////////////////

void CommitDetailsTest::test_data()
{
  CommitDetails cd;

  cd.set_option("Option1", "OldVal1", "NewVal1");
  cd.set_option("Option2", "OldVal2", "");
  cd.set_option("Option3", "", "NewVal3");

  // 1. invalid index
  QVERIFY( !cd.data(QModelIndex(), Qt::DecorationRole).isValid() );

  // 2. everything is ok (row 0)
  QCOMPARE( cd.data( cd.index(0, 0), Qt::DisplayRole).toString(), QString("Option1") );
  QCOMPARE( cd.data( cd.index(0, 1), Qt::DisplayRole).toString(), QString("\"OldVal1\"") );
  QCOMPARE( cd.data( cd.index(0, 2), Qt::DisplayRole).toString(), QString("\"NewVal1\"") );

  // 3. new value is empty (row 1)
  QCOMPARE( cd.data( cd.index(1, 0), Qt::DisplayRole).toString(), QString("Option2") );
  QCOMPARE( cd.data( cd.index(1, 1), Qt::DisplayRole).toString(), QString("\"OldVal2\"") );
  QCOMPARE( cd.data( cd.index(1, 2), Qt::DisplayRole).toString(), QString("--") );

  // 4. old value is empty (row 2)
  QCOMPARE( cd.data( cd.index(2, 0), Qt::DisplayRole).toString(), QString("Option3") );
  QCOMPARE( cd.data( cd.index(2, 1), Qt::DisplayRole).toString(), QString("--") );
  QCOMPARE( cd.data( cd.index(2, 2), Qt::DisplayRole).toString(), QString("\"NewVal3\"") );

  // 5. wrong rolw
  QVERIFY( !cd.data( cd.index(0, 0), Qt::DecorationRole).isValid() );
}

//////////////////////////////////////////////////////////////////////////

void CommitDetailsTest::test_headerData()
{
  CommitDetails cd;

  // 1. check column headers
  QCOMPARE( cd.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Name") );
  QCOMPARE( cd.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Old Value") );
  QCOMPARE( cd.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("New Value") );

  // 2. check row headers
  QCOMPARE( cd.headerData(0, Qt::Vertical, Qt::DisplayRole).toString(), QString("Option #1") );
  QCOMPARE( cd.headerData(14, Qt::Vertical, Qt::DisplayRole).toString(), QString("Option #15") );
  QCOMPARE( cd.headerData(8790, Qt::Vertical, Qt::DisplayRole).toString(), QString("Option #8791") );

  // 3. wrong column number
  QVERIFY( !cd.headerData(3, Qt::Horizontal, Qt::DisplayRole).isValid() );

  // 4. wrong role
  QVERIFY( !cd.headerData(1, Qt::Horizontal, Qt::DecorationRole).isValid() );
}

//////////////////////////////////////////////////////////////////////////

void CommitDetailsTest::test_index()
{
  CommitDetails cd;

  QVERIFY( !cd.index(0, 0).isValid() );

  cd.set_option("Option1", "OldVal", "NewVal");

  QModelIndex index = cd.index(0, 0);
  CommitDetailsItem * item = static_cast<CommitDetailsItem*>(index.internalPointer());
  QVERIFY( is_not_null(item) );

  QCOMPARE( item->option_name(), QString("Option1") );
  QCOMPARE( item->old_value(), QString("OldVal") );
  QCOMPARE( item->current_value(), QString("NewVal") );

  QVERIFY( !cd.index(0, 0, index).isValid() );
}

//////////////////////////////////////////////////////////////////////////

void CommitDetailsTest::test_setOption()
{
  CommitDetails cd;

  QVERIFY( !cd.has_options() );
  cd.set_option("Option1", "OldVal", "NewVal");
  QVERIFY( cd.has_options() );
}

//////////////////////////////////////////////////////////////////////////

void CommitDetailsTest::test_clear()
{
  CommitDetails cd(nullptr, "//Root/Path/To/Node");

  QVERIFY( !cd.has_options() );

  QCOMPARE( cd.node_path(), QString("//Root/Path/To/Node") );
  cd.set_option("Option1", "OldVal", "NewVal");

  QVERIFY( cd.has_options() );

  cd.clear();

  QVERIFY( !cd.has_options() );
  QVERIFY( cd.node_path().isEmpty() );
}

//////////////////////////////////////////////////////////////////////////

} // CoreTest
} // UI
} // cf3
