// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "GUI/Server/RemoteClientAppender.hpp"

using namespace CF::Common;
using namespace CF::GUI::Server;

RemoteClientAppender::RemoteClientAppender() : LogStringForwarder()
{}


void RemoteClientAppender::message(const std::string & data)
{
  emit newData( data.c_str() );
}


