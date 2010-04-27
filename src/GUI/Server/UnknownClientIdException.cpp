#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Server/UnknownClientIdException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

UnknownClientIdException::UnknownClientIdException(const CodeLocation& where,
                                                   const std::string& what)
: Exception(where, what, "UnknownClientIdException")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

UnknownClientIdException::UnknownClientIdException(const UnknownClientIdException& e) throw()
: Exception(e)
{

}
