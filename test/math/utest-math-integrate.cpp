// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

#include <boost/test/unit_test.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include "fparser/fparser.hh"

#include "common/Log.hpp"

#include "math/Integrate.hpp"
#include "math/Consts.hpp"

using namespace std;
using namespace boost;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;

struct f
{
  Real operator()(Real x) const
  {
    return std::sin(x);
  }
};

BOOST_AUTO_TEST_SUITE( math_integrate_test_suite )

BOOST_AUTO_TEST_CASE( test_integrate )
{
  Integrate integrate;
  Real result = integrate(f(), 0., 1.);
  CFinfo << "result = " << result << CFendl;
  CFinfo << "nb_intervals = " << integrate.nb_intervals << CFendl;
  CFinfo << "nb_evaluations = " << integrate.nb_kernel_evaluations << CFendl;
  CFinfo << "error_estimate = " << integrate.error_estimate << CFendl;
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()


#if 0
class fparser_integral:
    public FunctionParser::FunctionWrapper
{
 public:

  const std::string& name() { return m_name; }

  fparser_integral(const std::string& name, const std::string& integrand, const std::string& vars) : m_name(name), m_integrand(integrand,vars), integrate() {}

  struct WrappedIntegrand
  {
    WrappedIntegrand(const std::string& integrand, const std::string& vars)
    {
      std::vector<std::string> deduced_vars;
      func.ParseAndDeduceVariables(integrand,deduced_vars);

      std::cout << "deduced = ";
      boost_foreach(std::string& deduced_var, deduced_vars)
        std::cout << deduced_var << "  ";
      std::cout << std::endl;

      std::stringstream parse_vars;
      std::vector<std::string> passed_vars; passed_vars.reserve(deduced_vars.size());
      boost::char_separator<char> sep(",");
      typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
      tokenizer tok (vars,sep);
      Uint idx = 0;
      for(tokenizer::iterator it=tok.begin(); it!=tok.end(); ++it)
      {
        if(idx>0) parse_vars << ",";
        parse_vars << *it;

        passed_vars.push_back(*it);
        var_idx[*it] = idx++;
      }

      std::vector<std::string> unknown_vars; unknown_vars.reserve(2);

      boost_foreach(const std::string& deduced_var, deduced_vars)
      {
        bool found = false;
        boost_foreach(const std::string& passed_var, passed_vars)
        {
          if ( equals( passed_var, deduced_var) )
            found = true;
        }
        if (found == false)
          unknown_vars.push_back(deduced_var);
      }

      std::cout << "unknown = ";
      boost_foreach(const std::string& unknown, unknown_vars)
        std::cout << unknown << "  ";
      std::cout<< std::endl;

      if(unknown_vars.size() != 2)
        throw(ParsingFailed(FromHere(),""));

      std::string* dummy_var;
      std::string* integral_end;
      if (unknown_vars[0].size() < unknown_vars[1].size())
      {
        if(idx>0) parse_vars << ",";
        parse_vars << unknown_vars[0];
        var_idx[unknown_vars[0]] = idx++;

        if(idx>0) parse_vars << ",";
        parse_vars << unknown_vars[1];
        var_idx[unknown_vars[1]] = idx++;
      }
      else
      {
        if(idx>0) parse_vars << ",";
        parse_vars << unknown_vars[1];
        var_idx[unknown_vars[1]] = idx++;

        if(idx>0) parse_vars << ",";
        parse_vars << unknown_vars[0];
        var_idx[unknown_vars[0]] = idx++;
      }

      variables.resize(var_idx.size());
      variables[variables.size()-1] = 1.;

      std::cout << "parse_vars = " << parse_vars.str() << std::endl;
      func.Parse(integrand, parse_vars.str());
    }

    Real operator()(Real x) const
    {
      variables[var_idx["a"]]=1.;
      variables[variables.size()-2] = x;
      Real result = func.Eval(&variables[0]);
      return result;
    }

    mutable FunctionParser func;
    mutable std::vector<Real> variables;
    mutable std::map<std::string,Uint> var_idx;
  };

  virtual double callFunction(const double* values)
  {
    // Perform the actual function call here, like:
    return integrate(m_integrand,values[0],values[1]);
  }

private:
  std::string m_name;
  Integrate integrate;
  WrappedIntegrand m_integrand;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( math_integrate_fparser_test_suite )

BOOST_AUTO_TEST_CASE( test_integrate_fparser )
{
  Integrate integrate;
  Real result = integrate(f(), 0., 1.);
  CFinfo << "result = " << result << CFendl;
  CFinfo << "nb_intervals = " << integrate.nb_intervals << CFendl;
  CFinfo << "nb_evaluations = " << integrate.nb_kernel_evaluations << CFendl;
  CFinfo << "error_estimate = " << integrate.error_estimate << CFendl;

  FunctionParser fparser;

  fparser_integral my_integral("my_integral","sin(a*x)*dx","a");
  fparser.AddFunctionWrapper(my_integral.name(), my_integral, 2);

  fparser.Parse("my_integral(0,1)","a");

  Real variables[1] = {1};
  Real parsed_result = fparser.Eval(variables);

  BOOST_CHECK_EQUAL(parsed_result,result);

//  fparser.Parse("integral(sin(int_x),int_min,int_max)","xmin","xmax","x");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
#endif

