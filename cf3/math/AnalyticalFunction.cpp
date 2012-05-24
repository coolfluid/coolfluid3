// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
    m_vars(""),
    m_nbvars(0),
    m_function("")
{
}

AnalyticalFunction::AnalyticalFunction( const std::string& func, const std::string& vars)
  : m_is_parsed(false),
    m_vars(""),
    m_nbvars(0),
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
  return m_nbvars;
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::set_variables(const std::string& vars)
{
  m_vars = vars;
  // count nb_vars
  boost::char_separator<char> sep(",");
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tok (vars,sep);

  m_nbvars = 0;
  for (tokenizer::iterator el=tok.begin(); el!=tok.end(); ++el, ++m_nbvars )
  {
    //  CFinfo << "var" << m_nbvars << " [" << *el << "]" << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::parse (const std::string& function)
{
  parse(function,m_vars);
}

////////////////////////////////////////////////////////////////////////////////

void AnalyticalFunction::parse (const std::string& function, const std::string& vars)
{
  clear();
  set_variables(vars);
  m_function = function;

  m_parser = boost::shared_ptr<FunctionParser>( new FunctionParser() );
  m_parser->AddConstant("pi", Consts::pi());

    // CFinfo << "Parsing Function: \'" << m_functions[i] << "\' Vars: \'" << m_vars << "\'\n" << CFendl;
  m_parser->Parse(m_function,m_vars);

  if ( m_parser->GetParseErrorType() !=  FunctionParser::FP_NO_ERROR )
  {
    std::string msg("ParseError in AnalyticalFunction::parse(): ");
    msg += " Error [" +std::string(m_parser->ErrorMsg()) + "]";
    msg += " Function [" + m_function + "]";
    msg += " Vars: ["    + m_vars + "]";
    throw common::ParsingFailed (FromHere(),msg);
  }
  m_is_parsed = true;
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

