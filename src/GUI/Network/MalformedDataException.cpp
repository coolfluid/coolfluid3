#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Network/MalformedData.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;

MalformedData::MalformedDataException(const CodeLocation& where,
                                               const std::string& what)
: Exception(where, what, "MalformedData")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MalformedData::MalformedDataException(const MalformedDataException& e) throw()
: Exception(e)
{

}
