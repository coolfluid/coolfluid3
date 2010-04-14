#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/NetworkException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

NetworkException::NetworkException(const CodeLocation& where,
                                   const std::string& what)
: Exception(where, what, "Network")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NetworkException::NetworkException(const NetworkException& e) throw()
: Exception(e)
{

}
