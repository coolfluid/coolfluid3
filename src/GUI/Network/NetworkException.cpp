#include <string>

#include "Common/CodeLocation.hh"

#include "GUI/Network/NetworkException.hh"

using namespace CF::Common;
using namespace CF::GUI::Network;

NetworkException::NetworkException(const CodeLocation& where, 
                                   const std::string& what) 
: Exception(where, what, "NetworkException") 
{
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NetworkException::NetworkException(const NetworkException& e) throw() 
: Exception(e) 
{
  
}