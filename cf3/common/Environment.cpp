// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/OptionT.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/LogLevel.hpp"
#include "common/Log.hpp"
#include "common/Environment.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Environment, Component, LibCommon > Environment_Builder;

////////////////////////////////////////////////////////////////////////////////

Environment::Environment ( const std::string& name) : Component ( name )
{
  // properties
  properties()["brief"] = std::string("Environment");
  properties()["description"] = std::string("Controls general behavior of coolfluid");

  // options
  options().add_option< OptionT<bool> >("only_cpu0_writes", true)
      .pretty_name("Only CP0 Writes")
      .description("If true, only processor P0 writes the log info to files. If false, all processors write.")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_only_cpu0_writes,this));

  options().add_option< OptionT<bool> >("assertion_throws", AssertionManager::instance().AssertionThrows)
      .pretty_name("Assertion Throws")
      .description("If true, failed assertions throw exceptions instead of aborting. (Only for Debug builds)")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_assertion_throws,this));

  options().add_option< OptionT<bool> >("assertion_backtrace", AssertionManager::instance().AssertionDumps)
      .pretty_name("Assertion Backtrace")
      .description("If true, failed assertions dump the backtrace. (Only for Debug builds)")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_assertion_backtrace,this));

  options().add_option< OptionT<bool> >("disable_assertions", ! AssertionManager::instance().DoAssertions)
      .pretty_name("Disable Assertions")
      .description("If true, assertions will be ignored. (Only for Debug builds)")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_disable_assertions,this));

  options().add_option< OptionT<bool> >("exception_outputs", ExceptionManager::instance().ExceptionOutputs)
      .pretty_name("Exception Outputs")
      .description("If true, raised exceptions output immediately, before being handled.")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_exception_outputs,this));

  options().add_option< OptionT<bool> >("exception_backtrace", ExceptionManager::instance().ExceptionDumps)
      .pretty_name("Exception Backtrace")
      .description("If true, raised exceptions dump immediately the backtrace, before being handled.")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_exception_backtrace,this));

  options().add_option< OptionT<bool> >("exception_aborts", ExceptionManager::instance().ExceptionAborts)
      .pretty_name("Exception Aborts")
      .description("If true, raised exceptions abort program immediately, before being handled.")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_exception_aborts,this));

  options().add_option< OptionT<bool> >("regist_signal_handlers", false)
      .pretty_name("Regist Signal Handlers")
      .description("If true, regist signal handlers")
      .mark_basic();

  options().add_option< OptionT<bool> >("verbose_events", false)
      .pretty_name("Verbose Events")
      .description("If true, events are verbose output.")
      .mark_basic();

  options().add_option< OptionT<bool> >("error_on_unused_config", false)
      .pretty_name("Error On Unused Config")
      .description("If true, signal error when some user provided config parameters are not used.")
      .mark_basic();

  options().add_option< OptionT<std::string> >("main_logger_file_name", std::string("output.log"))
      .pretty_name("Main Logger File Name")
      .description("The name if the file in which to put the logging messages.")
      .mark_basic();

  options().add_option< OptionT<Uint> >("exception_log_level", (Uint) ERROR)
      .pretty_name("Exception Log Level")
      .description("The log level for exceptions")
      .mark_basic();

  options().add_option< OptionT<Uint> >("log_level", 3)
      .pretty_name("Log Level")
      .description("The log level [SILENT=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, TRACE=5, VERBOSE=10")
      .mark_basic()
      .attach_trigger(boost::bind(&Environment::trigger_log_level,this));

  trigger_log_level();

  // signals
  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
}

////////////////////////////////////////////////////////////////////////////////

Environment::~Environment()
{
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_only_cpu0_writes()
{
  bool opt = options().option("only_cpu0_writes").value<bool>();

  CFerror.setFilterRankZero( opt );
  CFwarn.setFilterRankZero( opt );
  CFinfo.setFilterRankZero( opt );
  CFdebug.setFilterRankZero( opt );
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_assertion_throws()
{
  AssertionManager::instance().AssertionThrows = options().option("assertion_throws").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_assertion_backtrace()
{
  AssertionManager::instance().AssertionDumps = options().option("assertion_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_disable_assertions()
{
  AssertionManager::instance().DoAssertions = ! options().option("disable_assertions").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_exception_outputs()
{
  ExceptionManager::instance().ExceptionOutputs = options().option("exception_outputs").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_exception_backtrace()
{
  ExceptionManager::instance().ExceptionDumps = options().option("exception_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_exception_aborts()
{
  ExceptionManager::instance().ExceptionAborts = options().option("exception_aborts").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void Environment::trigger_log_level()
{
  Logger::instance().set_log_level(options().option("log_level").value<Uint>());
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
