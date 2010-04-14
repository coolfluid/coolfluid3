#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/IOException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

IOException::IOException(const CodeLocation& where, const std::string& what)
: Exception(where, what, "IO")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

IOException::IOException(const IOException& e) throw()
: Exception(e)
{

}
