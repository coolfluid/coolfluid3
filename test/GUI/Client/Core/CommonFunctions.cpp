// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QTest>

#include "rapidxml/rapidxml.hpp"

#include "Common/XML/FileOperations.hpp"

#include "UI/Core/CNode.hpp"

#include "test/GUI/Client/Core/ExceptionThrowHandler.hpp"
#include "test/GUI/Client/Core/CommonFunctions.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::Core;
using namespace CF::UI::ClientTest;

NRoot::Ptr CF::UI::ClientTest::makeTreeFromFile()
{
  static XmlDoc::Ptr doc = XML::parse_file(boost::filesystem::path("./tree.xml"));

  static NRoot::Ptr root = CNode::createFromXml(doc->content->first_node())->castTo<NRoot>();
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
