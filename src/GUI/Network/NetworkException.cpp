#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/Network.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

Network::NetworkException(const CodeLocation& where,
                                   const std::string& what)
: Exception(where, what, "Network")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Network::NetworkException(const NetworkException& e) throw()
: Exception(e)
{

}
