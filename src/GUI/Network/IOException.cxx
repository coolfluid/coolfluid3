#include <string>

#include "Common/CodeLocation.hh"

#include "GUI/Network/IOException.hh"

using namespace CF::Common;
using namespace CF::GUI::Network;

IOException::IOException(const CodeLocation& where, const std::string& what) 
: Exception(where, what, "IOException") 
{
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

IOException::IOException(const IOException& e) throw() 
: Exception(e) 
{
  
}