#include <string>

#include "Common/CodeLocation.hh"

#include "GUI/Network/MalformedDataException.hh"

using namespace CF::Common;
using namespace CF::GUI::Network;

MalformedDataException::MalformedDataException(const CodeLocation& where, 
                                               const std::string& what) 
: Exception(where, what, "MalformedDataException") 
{
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MalformedDataException::MalformedDataException(const MalformedDataException& e) throw() 
: Exception(e) 
{
  
}