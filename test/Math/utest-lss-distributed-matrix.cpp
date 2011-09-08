// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Math::LSS where testing on a distributed matrix."

////////////////////////////////////////////////////////////////////////////////

#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/System.hpp"
#include "test/Math/utest-lss-test-matrix.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;
using namespace CF::Math::LSS;

////////////////////////////////////////////////////////////////////////////////

struct LSSDistributedMatrixFixture
{
  /// common setup for each test case
  LSSDistributedMatrixFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~LSSDistributedMatrixFixture()
  {
  }

  /// common params
  int m_argc;
  char** m_argv;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LSSDistributedMatrixSuite, LSSDistributedMatrixFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_parallel_environment )
{
  Common::Comm::PE::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(Common::Comm::PE::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( system_solve )
{
  // test matrix
  test_matrix m;

  // commpattern
  Common::Comm::CommPattern cp("commpattern");
  cp.insert("gid",m.global_numbering,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<Common::CommWrapper>(),m.irank_updatable);

  // system
  LSS::System sys("system");
  sys.options().option("solver").change_value(boost::lexical_cast<std::string>("Trilinos"));
  sys.create(cp,m.nbeqs,m.column_indices,m.rowstart_positions);
  sys.reset();

  // filling the system
  Real* vals=&m.mat_presolve[0];
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    if (cp.isUpdatable()[i])
      for (int j=0; j<(const int)m.nbeqs; j++)
        for (int k=m.rowstart_positions[i]; k<(const int)(m.rowstart_positions[i+1]); k++)
          for (int l=0; l<(const int)m.nbeqs; l++)
            sys.matrix()->set_value(m.column_indices[k]*m.nbeqs+l,i*m.nbeqs+j,*vals++);
  vals=&m.rhs_presolve[0];
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    for (int j=0; j<(const int)m.nbeqs; j++)
      sys.rhs()->set_value(i,j,*vals++);
  vals=&m.sol_presolve[0];
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    for (int j=0; j<(const int)m.nbeqs; j++)
      sys.solution()->set_value(i,j,*vals++);

  // write a settings file for trilinos, solving with plain bicgstab, no preconditioning
  if (m.irank==0)
  {
    std::ofstream trilinos_xml("trilinos_settings.xml");
    trilinos_xml << "<ParameterList>\n";
    trilinos_xml << "  <Parameter name=\"Linear Solver Type\" type=\"string\" value=\"AztecOO\"/>\n";
    trilinos_xml << "  <ParameterList name=\"Linear Solver Types\">\n";
    trilinos_xml << "    <ParameterList name=\"AztecOO\">\n";
    trilinos_xml << "      <ParameterList name=\"Forward Solve\">\n";
    trilinos_xml << "        <ParameterList name=\"AztecOO Settings\">\n";
    trilinos_xml << "          <Parameter name=\"Aztec Solver\" type=\"string\" value=\"BiCGStab\"/>\n";
    trilinos_xml << "        </ParameterList>\n";
    trilinos_xml << "        <Parameter name=\"Max Iterations\" type=\"int\" value=\"5000\"/>\n";
    trilinos_xml << "        <Parameter name=\"Tolerance\" type=\"double\" value=\"1e-13\"/>\n";
    trilinos_xml << "      </ParameterList>\n";
    trilinos_xml << "    </ParameterList>\n";
    trilinos_xml << "  </ParameterList>\n";
    trilinos_xml << "  <Parameter name=\"Preconditioner Type\" type=\"string\" value=\"None\"/>\n";
    trilinos_xml << "</ParameterList>\n";
    trilinos_xml.close();
  }
  Common::Comm::PE::instance().barrier();

  sys.print("distributed_system_" + boost::lexical_cast<std::string>(m.irank) + ".plt");

  // and solve the system
  sys.solve();

  // check results
  std::vector<Real> v;
  sys.solution()->data(v);
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    if (cp.isUpdatable()[i])
      for (int j=0; j<(const int)m.nbeqs; j++)
        BOOST_CHECK_CLOSE(v[i*m.nbeqs+j],m.result[i*m.nbeqs+j],1e-3);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_parallel_environment )
{
  CFinfo.setFilterRankZero(true);
  Common::Comm::PE::instance().finalize();
  BOOST_CHECK_EQUAL(Common::Comm::PE::instance().is_active(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

