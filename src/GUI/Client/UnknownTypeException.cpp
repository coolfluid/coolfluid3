#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Client/UnknownTypeException.hpp"

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
