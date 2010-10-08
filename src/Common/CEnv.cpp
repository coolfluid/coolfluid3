// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/OptionT.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"
#include "Common/XmlHelpers.hpp"
#include "Common/LogLevel.hpp"

#include "Common/CEnv.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CEnv, Component, LibCommon, NB_ARGS_1 >
CEnv_Provider ( CEnv::type_name() );

////////////////////////////////////////////////////////////////////////////////

CEnv::CEnv ( const CName& name) : Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CEnv::~CEnv()
{
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::defineConfigProperties ( Common::PropertyList& options )
{
  options.add_option< OptionT<bool> >("OnlyCP0Writes", "If true, only processor P0 writes the log info to files. If false, all processors write.", true);
  options.add_option< OptionT<bool> >("AssertionThrows", "If true, failed assertions throw exceptions instead of abording.", false); /// @todo is it ok ?
  options.add_option< OptionT<bool> >("RegistRignalHandlers", "If true, regist signal handlers", true);
  options.add_option< OptionT<bool> >("TraceActive", "If true, trace is active.", false);
  options.add_option< OptionT<bool> >("TraceToStdOut", "If true, trace log is also written to the stdout.", false);
  options.add_option< OptionT<bool> >("VerboseEvents", "If true, events are verbose output.", false);
  options.add_option< OptionT<bool> >("ErrorOnUnusedConfig", "If true, signal error when some user provided config parameters are not used.", false);
  options.add_option< OptionT<std::string> >("MainLoggerFileName", "The name if the file in which to put the logging messages.", "output.log");
  options.add_option< OptionT<Uint> >("ExceptionLogLevel", "The log level for exceptions", (Uint) VERBOSE);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
