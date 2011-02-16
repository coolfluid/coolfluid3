// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/Log.hpp"

#include "Common/URI.hpp"
#include "Common/XmlHelpers.hpp"

using namespace CF::Common;

void test_signal( XmlNode & frame )
{
  XmlNode& reply = *XmlOps::add_reply_frame( frame );
  XmlParams preply(reply);
  XmlParams pframe(frame);

  std::vector<int> vect = boost::assign::list_of<int>(235)(45465)(13213)(454);

  CFinfo << "/////////////// in test_signal() \\\\\\\\\\\\\\\\" << CFendl << CFendl;

  CFinfo << "OptionOne value is " << pframe.get_option<int>("OptionOne") << CFendl;
  CFinfo << "OptionTwo value is " << pframe.get_option<std::string>("OptionTwo") << CFendl;
  CFinfo << "OptionThree value is " << pframe.get_option<CF::Real>("OptionThree") << CFendl;

  preply.add_array( "ArrayOne", vect, " ; ");
  preply.add_option( "OptionFour", CF::Real(3.1415));

  CFinfo << CFendl << "/////////////// leaving test_signal() \\\\\\\\\\\\\\\\" << CFendl << CFendl;
}

int main ( int argc, char * argv[])
{
  std::string str;
  URI sender("//Root/Sender", URI::Scheme::CPATH);
  URI receiver("//Root/Receiver", URI::Scheme::CPATH);
  std::vector<int> vect;

  boost::shared_ptr<XmlDoc> xmldoc = XmlOps::create_doc();
  XmlNode & doc_node = *XmlOps::goto_doc_node(*xmldoc.get());

  XmlNode & frame = *XmlOps::add_signal_frame(doc_node, "MySignal", sender, receiver, true);

  XmlParams p(frame);

  p.add_option( "OptionOne", int(212135));
  p.add_option( "OptionTwo", std::string("Hello, World!"));
  p.add_option( "OptionThree", CF::Real(2.71));

  // output the signal
  XmlOps::xml_to_string(*xmldoc.get(), str);
  CFinfo << str << CFendl << CFendl;

  test_signal( frame );

  // output the signal and its reply
  str.clear();
  XmlOps::xml_to_string(*xmldoc.get(), str);
  CFinfo << str << CFendl << CFendl;

  XmlNode & reply = *frame.next_sibling("frame");
  XmlParams preply(reply);

  vect = preply.get_array<int>("ArrayOne");

  for(int i = 0 ; i < vect.size() ; ++i)
    CFinfo << "ArrayOne[" << i << "] value is " << vect[i] << CFendl;

  CFinfo << "OptionFour value is " << preply.get_option<CF::Real>("OptionFour") << CFendl;

  return 0;
}

