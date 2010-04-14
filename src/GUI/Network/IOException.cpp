#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/IO.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

IO::IOException(const CodeLocation& where, const std::string& what)
: Exception(where, what, "IO")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

IO::IOException(const IOException& e) throw()
: Exception(e)
{

}
