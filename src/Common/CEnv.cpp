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
  m_properties["brief"] = std::string("Environment");
  m_properties["description"] = std::string("Controls general behavior of coolfluid");

  // properties
  m_properties.add_option(OptionT<bool>::create("only_cpu0_writes","Only CP0 Writes", "If true, only processor P0 writes the log info to files. If false, all processors write.", true))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_only_cpu0_writes,this));

  m_properties.add_option(OptionT<bool>::create("assertion_to_exception","Assertion To Exception", "If true, failed assertions throw exceptions instead of aborting. (Only for Debug builds)", AssertionManager::instance().AssertionThrows))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_assertion_throws,this));

  m_properties.add_option(OptionT<bool>::create("assertion_backtrace","Assertion Backtrace", "If true, failed assertions dump the backtrace. (Only for Debug builds)", AssertionManager::instance().AssertionDumps))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_assertion_backtrace,this));

  m_properties.add_option(OptionT<bool>::create("disable_assertions","Disable Assertions", "If true, assertions will be ignored. (Only for Debug builds)", ! AssertionManager::instance().DoAssertions))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_disable_assertions,this));

  m_properties.add_option(OptionT<bool>::create("exception_outputs","Exception Outputs", "If true, raised exceptions output immediately, before being handled.", ExceptionManager::instance().ExceptionOutputs))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_exception_outputs,this));

  m_properties.add_option(OptionT<bool>::create("exception_backtrace","Exception Backtrace", "If true, raised exceptions dump immediately the backtrace, before being handled.", ExceptionManager::instance().ExceptionDumps))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_exception_backtrace,this));

  m_properties.add_option(OptionT<bool>::create("exception_aborts","Exception Aborts", "If true, raised exceptions abort program immediately, before being handled.", ExceptionManager::instance().ExceptionAborts))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_exception_aborts,this));

  m_properties.add_option(OptionT<bool>::create("trace_active","Trace Active", "If true, trace is active.", false))
    ->mark_basic()
    ->attach_trigger(boost::bind(&CEnv::trigger_trace_active,this));

  m_properties.add_option(OptionT<bool>::create("trace_to_std_out","Trace To Std Out", "If true, trace log is also written to the stdout.", true))
    ->mark_basic();

  m_properties.add_option(OptionT<bool>::create("regist_signal_handlers","Regist Signal Handlers", "If true, regist signal handlers", true))
    ->mark_basic();

  m_properties.add_option(OptionT<bool>::create("verbose_events","Verbose Events", "If true, events are verbose output.", false))
    ->mark_basic();

  m_properties.add_option(OptionT<bool>::create("error_on_unused_config","ErrorOnUnusedConfig", "If true, signal error when some user provided config parameters are not used.", false))
    ->mark_basic();

  m_properties.add_option(OptionT<std::string>::create("main_logger_file_name","Main Logger File Name", "The name if the file in which to put the logging messages.", "output.log"))
    ->mark_basic();

  m_properties.add_option(OptionT<Uint>::create("exception_log_level","Exception Log Level", "The log level for exceptions", (Uint) ERROR))
    ->mark_basic();

  m_properties.add_option(OptionT<Uint>::create("log_level","Log Level", "The log level [SILENT=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, TRACE=5, VERBOSE=10", 3))
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
  CFerror.setFilterRankZero(property("only_cpu0_writes").value<bool>());
  CFwarn.setFilterRankZero(property("only_cpu0_writes").value<bool>());
  CFinfo.setFilterRankZero(property("only_cpu0_writes").value<bool>());
  CFdebug.setFilterRankZero(property("only_cpu0_writes").value<bool>());
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_assertion_throws()
{
  AssertionManager::instance().AssertionThrows = property("assertion_to_exception").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_assertion_backtrace()
{
  AssertionManager::instance().AssertionDumps = property("assertion_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_disable_assertions()
{
  AssertionManager::instance().DoAssertions = ! property("disable_assertions").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_outputs()
{
  ExceptionManager::instance().ExceptionOutputs = property("exception_outputs").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_backtrace()
{
  ExceptionManager::instance().ExceptionDumps = property("exception_backtrace").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_exception_aborts()
{
  ExceptionManager::instance().ExceptionAborts = property("exception_aborts").value<bool>();
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_log_level()
{
  Logger::instance().set_log_level(property("log_level").value<Uint>());
}

////////////////////////////////////////////////////////////////////////////////

void CEnv::trigger_trace_active()
{
  CFtrace.set_log_level(property("trace_active").value<bool>() ? TRACE : SILENT);
}


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
