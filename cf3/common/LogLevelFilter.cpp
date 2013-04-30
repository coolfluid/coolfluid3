// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/LogLevelFilter.hpp"

using namespace cf3;
using namespace cf3::common;

LogLevelFilter::LogLevelFilter(LogLevel level)
: m_filter(level),
  m_log_level(level),
  m_tmp_log_level(level)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::set_filter(LogLevel level)
{
  m_filter = level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogLevel LogLevelFilter::get_filter() const
{
  return m_filter;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::set_log_level(const Uint level)
{
  m_log_level = level;
  m_tmp_log_level = m_log_level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Uint LogLevelFilter::get_log_level() const
{
  return m_log_level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::resetToDefaultLevel()
{
  m_tmp_log_level = m_log_level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::set_tmp_log_level(const Uint level)
{
  m_tmp_log_level = level;
}
