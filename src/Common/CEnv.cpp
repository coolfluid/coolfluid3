// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionT.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/LogLevel.hpp"
#include "Common/Log.hpp"
#include "Common/CEnv.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CEnv, Component, LibCommon > CEnv_Builder;

////////////////////////////////////////////////////////////////////////////////

CEnv::CEnv ( const std::string& name) : Component ( name )
{
  // properties
  m_properties["brief"] = std::string("Environment");
  m_properties["description"] = std::string("Controls general behavior of coolfluid");

  // options
  m_options.add_option< OptionT<bool> >("only_cpu0_writes", true)
      ->set_pretty_name("Only CP0 Writes")
      ->set_description("If true, only processor P0 writes the log info to files. If false, all processors write.")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_only_cpu0_writes,this));

  m_options.add_option< OptionT<bool> >("assertion_throws", AssertionManager::instance().AssertionThrows)
      ->set_pretty_name("Assertion Throws")
      ->set_description("If true, failed assertions throw exceptions instead of aborting. (Only for Debug builds)")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_assertion_throws,this));

  m_options.add_option< OptionT<bool> >("assertion_backtrace", AssertionManager::instance().AssertionDumps)
      ->set_pretty_name("Assertion Backtrace")
      ->set_description("If true, failed assertions dump the backtrace. (Only for Debug builds)")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_assertion_backtrace,this));

  m_options.add_option< OptionT<bool> >("disable_assertions", ! AssertionManager::instance().DoAssertions)
      ->set_pretty_name("Disable Assertions")
      ->set_description("If true, assertions will be ignored. (Only for Debug builds)")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_disable_assertions,this));

  m_options.add_option< OptionT<bool> >("exception_outputs", ExceptionManager::instance().ExceptionOutputs)
      ->set_pretty_name("Exception Outputs")
      ->set_description("If true, raised exceptions output immediately, before being handled.")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_exception_outputs,this));

  m_options.add_option< OptionT<bool> >("exception_backtrace", ExceptionManager::instance().ExceptionDumps)
      ->set_pretty_name("Exception Backtrace")
      ->set_description("If true, raised exceptions dump immediately the backtrace, before being handled.")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_exception_backtrace,this));

  m_options.add_option< OptionT<bool> >("exception_aborts", ExceptionManager::instance().ExceptionAborts)
      ->set_pretty_name("Exception Aborts")
      ->set_description("If true, raised exceptions abort program immediately, before being handled.")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_exception_aborts,this));

  m_options.add_option< OptionT<bool> >("regist_signal_handlers", false)
      ->set_pretty_name("Regist Signal Handlers")
      ->set_description("If true, regist signal handlers")
      ->mark_basic();

  m_options.add_option< OptionT<bool> >("verbose_events", false)
      ->set_pretty_name("Verbose Events")
      ->set_description("If true, events are verbose output.")
      ->mark_basic();

  m_options.add_option< OptionT<bool> >("error_on_unused_config", false)
      ->set_pretty_name("Error On Unused Config")
      ->set_description("If true, signal error when some user provided config parameters are not used.")
      ->mark_basic();

  m_options.add_option< OptionT<std::string> >("main_logger_file_name", std::string("output.log"))
      ->set_pretty_name("Main Logger File Name")
      ->set_description("The name if the file in which to put the logging messages.")
      ->mark_basic();

  m_options.add_option< OptionT<Uint> >("exception_log_level", (Uint) ERROR)
      ->set_pretty_name("Exception Log Level")
      ->set_description("The log level for exceptions")
      ->mark_basic();

  m_options.add_option< OptionT<Uint> >("log_level", 3)
      ->set_pretty_name("Log Level")
      ->set_description("The log level [SILENT=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, TRACE=5, VERBOSE=10")
      ->mark_basic()
      ->attach_trigger(boost::bind(&CEnv::trigger_log_level,this));

  trigger_log_level();

  // signals
  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;
}

////////////////////////////////////////////////////////////////////////////////

CEnv::~CEnv()
{
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_only_cpu0_writes()
{
  CFerror.setFilterRankZero(option("only_cpu0_writes").value<bool>());
  CFwarn.setFilterRankZero(option("only_cpu0_writes").value<bool>());
  CFinfo.setFilterRankZero(option("only_cpu0_writes").value<bool>());
  CFdebug.setFilterRankZero(option("only_cpu0_writes").value<bool>());
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_assertion_throws()
{
  AssertionManager::instance().AssertionThrows = option("assertion_throws").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_assertion_backtrace()
{
  AssertionManager::instance().AssertionDumps = option("assertion_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_disable_assertions()
{
  AssertionManager::instance().DoAssertions = ! option("disable_assertions").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_outputs()
{
  ExceptionManager::instance().ExceptionOutputs = option("exception_outputs").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_backtrace()
{
  ExceptionManager::instance().ExceptionDumps = option("exception_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_aborts()
{
  ExceptionManager::instance().ExceptionAborts = option("exception_aborts").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_log_level()
{
  Logger::instance().set_log_level(option("log_level").value<Uint>());
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
