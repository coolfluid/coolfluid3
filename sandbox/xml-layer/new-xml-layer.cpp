// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/Log.hpp"

#include "Common/XML/SignalFrame.hpp" // indirectly includes XmlNode.hpp

using namespace CF::Common;
using namespace CF::Common::XML;

void test_signal( SignalFrame & frame )
{
  SignalFrame reply = frame.create_reply();
  SignalFrame & roptions = reply.map("options");
  SignalFrame & foptions = frame.map("options");

  std::vector<int> vect = boost::assign::list_of<int>(235)(45465)(13213)(454);

  CFinfo << "/////////////// in test_signal() \\\\\\\\\\\\\\\\" << CFendl << CFendl;

  CFinfo << "OptionOne value is " << foptions.get_option<int>("OptionOne") << CFendl;
  CFinfo << "OptionTwo value is " << foptions.get_option<std::string>("OptionTwo") << CFendl;
  CFinfo << "OptionThree value is " << foptions.get_option<CF::Real>("OptionThree") << CFendl;

  roptions.set_array( "ArrayOne", vect, " ; ");
  roptions.set_option( "OptionFour", CF::Real(3.1415));

  CFinfo << CFendl << "/////////////// leaving test_signal() \\\\\\\\\\\\\\\\" << CFendl << CFendl;
}

int main ( int argc, char * argv[])
{
  std::string str;
  URI sender("//Root/Sender", URI::Scheme::CPATH);
  URI receiver("//Root/Receiver", URI::Scheme::CPATH);
  std::vector<int> vect;

  SignalFrame signal_frame("MySignal",  sender, receiver);
  SignalFrame & foptions = signal_frame.map("options");

  foptions.set_option( "OptionOne", int(212135));
  foptions.set_option( "OptionTwo", std::string("Hello, World!"));
  foptions.set_option( "OptionThree", CF::Real(2.71));

  // output the signal
  signal_frame.xml_doc->to_string(str);
  CFinfo << str << CFendl << CFendl;

  test_signal( signal_frame );

  // output the signal and its reply
  str.clear();
  signal_frame.xml_doc->to_string(str);
  CFinfo << str << CFendl;

  SignalFrame reply_frame = signal_frame.get_reply();
  SignalFrame & roptions = reply_frame.map("options");

  vect = roptions.get_array<int>("ArrayOne");

  for(int i = 0 ; i < vect.size() ; ++i)
    CFinfo << "ArrayOne[" << i << "] value is " << vect[i] << CFendl;

  CFinfo << "OptionFour value is " << roptions.get_option<CF::Real>("OptionFour") << CFendl;

  return 0;
}

