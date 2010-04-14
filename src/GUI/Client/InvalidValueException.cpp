#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Client/InvalidValueException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

InvalidValueException::InvalidValueException(const CodeLocation& where,
                                             const std::string& what)
: Exception(where, what, "InvalidValue")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

InvalidValueException::InvalidValueException(const InvalidValueException& e) throw()
: Exception(e)
{

}
