#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Client/InvalidValueException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;

InvalidValue::InvalidValue(const CodeLocation& where,
                                             const std::string& what)
: Exception(where, what, "InvalidValue")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

InvalidValue::InvalidValue(const InvalidValue& e) throw()
: Exception(e)
{

}
