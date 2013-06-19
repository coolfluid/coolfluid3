// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tokenizer.hpp>

#include "common/Log.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"

#include "math/AnalyticalFunction.hpp"
#include "math/Consts.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

////////////////////////////////////////////////////////////////////////////////

AnalyticalFunction::AnalyticalFunction()
  : m_is_parsed(false),
    m_vars(),
    m_function("")
{
}

AnalyticalFunction::AnalyticalFunction( const std::string& func, const std::string& vars, const std::string& separator)
  : m_is_parsed(false),
    m_vars(),
    m_function("")
{
  parse(func,vars,separator);
}

AnalyticalFunction::AnalyticalFunction( const std::string& func, const std::vector<std::string>& vars )
  : m_is_parsed(false),
    m_vars(),
    m_function("")
{
  parse(func,vars);
}

////////////////////////////////////////////////////////////////////////////////

AnalyticalFunction::~AnalyticalFunction()
{
  clear();
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::clear()
{
  m_parser.reset();
  m_is_parsed = false;
}

////////////////////////////////////////////////////////////////////////////////

Uint AnalyticalFunction::nbvars() const
{
  cf3_assert ( is_parsed() );
  return m_vars.size();
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::set_variables(const std::string& vars, const std::string& separator)
{
  boost::char_separator<char> sep(separator.c_str());
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tok (vars,sep);
  m_vars.clear();
  for (tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el)
  {
    m_vars.push_back(*el);
    //  CFinfo << "var" << m_nbvars << " [" << *el << "]" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::set_variables(const std::vector<std::string>& vars)
{
  m_vars = vars;
}


////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::parse (const std::string& function)
{
  clear();
  m_function = function;

  m_parser = boost::shared_ptr<FunctionParser>( new FunctionParser() );
  m_parser->AddConstant("pi", Consts::pi());

    // CFinfo << "Parsing Function: \'" << m_functions[i] << "\' Vars: \'" << m_vars << "\'\n" << CFendl;
  std::stringstream ss;
  for (Uint i=0; i<m_vars.size(); ++i)
  {
    if (i!=0) ss << ",";
    ss << m_vars[i];
  }
  const int r = m_parser->Parse(m_function,ss.str());

  if ( m_parser->GetParseErrorType() != FunctionParser::FP_NO_ERROR )
  {
    std::string msg("ParseError in AnalyticalFunction::parse():\n");
    msg += " Function [" + m_function + "]\n";
    msg += std::string(r+11, ' ') + "^ " + std::string(m_parser->ErrorMsg()) + "\n";
    msg += " Vars     [" + ss.str() + "]\n";
    throw common::ParsingFailed (FromHere(),msg);
  }
  m_is_parsed = true;
}

void AnalyticalFunction::parse_and_deduce_variables (const std::string& function, std::vector<std::string>& vars)
{
  clear();
  m_function = function;

  m_parser = boost::shared_ptr<FunctionParser>( new FunctionParser() );
  m_parser->AddConstant("pi", Consts::pi());

  const int r = m_parser->ParseAndDeduceVariables(m_function, vars);
  set_variables(vars);
  std::stringstream ss;
  for (Uint i=0; i<m_vars.size(); ++i)
  {
    if (i!=0) ss << ",";
    ss << m_vars[i];
  }

  if ( m_parser->GetParseErrorType() != FunctionParser::FP_NO_ERROR )
  {
    std::string msg("ParseError in AnalyticalFunction::parseAndDeduceVariables():\n");
    msg += " Function [" + m_function + "]\n";
    msg += std::string(r+11, ' ') + "^ " + std::string(m_parser->ErrorMsg()) + "\n";
    msg += " Vars     [" + ss.str() + "]\n";
    throw common::ParsingFailed (FromHere(),msg);
  }
  m_is_parsed = true;
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::parse (const std::string& function, const std::vector<std::string>& vars)
{
  set_variables(vars);
  parse(function);
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::parse (const std::string& function, const std::string& vars, const std::string& separator)
{
  set_variables(vars,separator);
  parse(function);
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

