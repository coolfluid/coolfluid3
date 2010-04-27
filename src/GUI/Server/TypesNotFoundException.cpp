#include <string>

#include "Common/CodeLocation.hpp"

#include "GUI/Server/TypesNotFoundException.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

TypesNotFoundException::TypesNotFoundException(const CodeLocation& where,
                                                   const std::string& what)
: Exception(where, what, "TypesNotFoundException")
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TypesNotFoundException::TypesNotFoundException(const TypesNotFoundException& e) throw()
: Exception(e)
{

}
