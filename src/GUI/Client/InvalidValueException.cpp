#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Client/InvalidValue.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

InvalidValue::InvalidValueException(const CodeLocation& where,
                                             const std::string& what)
: Exception(where, what, "InvalidValue")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

InvalidValue::InvalidValueException(const InvalidValueException& e) throw()
: Exception(e)
{

}
