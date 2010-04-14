#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Client/UnknownType.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

UnknownType::UnknownTypeException(const CodeLocation& where,
                                           const std::string& what)
: Exception(where, what, "UnknownType")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

UnknownType::UnknownTypeException(const UnknownTypeException& e) throw()
: Exception(e)
{

}
