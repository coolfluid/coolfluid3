// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
