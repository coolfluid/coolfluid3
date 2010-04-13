#include <string>

#include "Common/CodeLocation.hh"

#include "GUI/Client/UnknownTypeException.hh"

using namespace CF::Common;
using namespace CF::GUI::Client;

UnknownTypeException::UnknownTypeException(const CodeLocation& where, 
                                           const std::string& what) 
: Exception(where, what, "UnknownTypeException") 
{
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

UnknownTypeException::UnknownTypeException(const UnknownTypeException& e) throw() 
: Exception(e) 
{
  
}