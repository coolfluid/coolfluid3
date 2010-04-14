#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/MalformedDataException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

MalformedDataException::MalformedDataException(const CodeLocation& where,
                                               const std::string& what)
: Exception(where, what, "MalformedData")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MalformedDataException::MalformedDataException(const MalformedDataException& e) throw()
: Exception(e)
{

}
