#include <string>

#include "Common/CodeLocation.hh"

#include "GUI/Client/InvalidValueException.hh"

using namespace CF::Common;
using namespace CF::GUI::Client;

InvalidValueException::InvalidValueException(const CodeLocation& where, 
                                             const std::string& what) 
: Exception(where, what, "InvalidValueException") 
{
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

InvalidValueException::InvalidValueException(const InvalidValueException& e) throw() 
: Exception(e) 
{
  
}