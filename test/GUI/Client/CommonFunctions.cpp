// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QTest>

#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/CNode.hpp"

#include "test/GUI/Client/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/CommonFunctions.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientTest;

NRoot::Ptr CF::GUI::ClientTest::makeTreeFromFile()
{
  static boost::shared_ptr<XmlDoc> doc = XmlOps::parse(boost::filesystem::path("./tree.xml"));

  static NRoot::Ptr root = CNode::createFromXml(*doc->first_node())->castTo<NRoot>();
  return root;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CommonFunctionsTest::test_makeTreeFromFile()
{
  // this is mainly to check whether the file is found (an exception is
  // thrown if not)
  GUI_CHECK_NO_THROW(makeTreeFromFile());
}
