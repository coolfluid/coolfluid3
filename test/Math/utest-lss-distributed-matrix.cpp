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
#include "Common/PE/Comm.hpp"
#include "Common/PE/CommPattern.hpp"
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
  Common::PE::Comm::instance().init(m_argc,m_argv);
  BOOST_CHECK_EQUAL(Common::PE::Comm::instance().is_active(),true);
  CFinfo.setFilterRankZero(false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( system_solve )
{
  // test matrix
  test_matrix m;

  // commpattern
  Common::PE::CommPattern::Ptr cp_ptr = Common::allocate_component<Common::PE::CommPattern>("commpattern");
  Common::PE::CommPattern& cp = *cp_ptr;
  cp.insert("gid",m.global_numbering,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<Common::PE::CommWrapper>(),m.irank_updatable);

  // system
  LSS::System::Ptr sys_ptr = Common::allocate_component<LSS::System>("system");
  LSS::System& sys = *sys_ptr;
  sys.options().option("solver").change_value(boost::lexical_cast<std::string>("Trilinos"));
  sys.create(cp,m.nbeqs,m.column_indices,m.rowstart_positions);
  sys.reset();

  // mimicing the assembly for performance measuring
  /*
    with az_msrmatrix and native written fill, the suminto part takes the following number of ticks in morpheus (for one step):
    process | assembly_ticks | matrixfill_ticks | setbc_ticks | solve_ticks
    -----------------------------------------------------------------------
    0       | 330499         | 239996           | 24888       | 295230350
    1       | 337050         | 248826           | 49497       | 306481891
    2       | 329032         | 242850           | 37709       | 298537437
    3       | 388584         | 280844           | 13722       | 299270122
  */
  BlockAccumulator ba;
  ba.resize(3,m.nbeqs);
  ba.reset(1.);
  for (int i=0; i<(const int)(m.elem_nodes.size()/3); i++)
  {
    ba.indices[0]=m.elem_nodes[3*i+0];
    ba.indices[1]=m.elem_nodes[3*i+1];
    ba.indices[2]=m.elem_nodes[3*i+2];
    sys.add_values(ba);
  }

sys.print("sys_test_assembly_bc_" + boost::lexical_cast<std::string>(m.irank) + ".plt");


  // fastly filling with pre-boundary condition values, THIS IS NOT THE WAY YOU NORMALLY ASSEMBLE
  Real* vals=&m.mat_prebc[0];
  sys.reset();
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    if (cp.isUpdatable()[i])
      for (int j=0; j<(const int)m.nbeqs; j++)
        for (int k=m.rowstart_positions[i]; k<(const int)(m.rowstart_positions[i+1]); k++)
          for (int l=0; l<(const int)m.nbeqs; l++)
            sys.matrix()->set_value(m.column_indices[k]*m.nbeqs+l,i*m.nbeqs+j,*vals++);
  vals=&m.rhs_prebc[0];
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    for (int j=0; j<(const int)m.nbeqs; j++)
      sys.rhs()->set_value(i,j,*vals++);
  vals=&m.sol_prebc[0];
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    for (int j=0; j<(const int)m.nbeqs; j++)
      sys.solution()->set_value(i,j,*vals++);

  // applying boundary conditions
  for (int i=0; i<(const int)m.bc_node.size(); i++)
    if (cp.isUpdatable()[m.bc_node[i]])
      sys.dirichlet(m.bc_node[i],m.bc_eqn[i],m.bc_value[i]);
  for (int i=0; i<(const int)m.periodic_pairs.size(); i+=2)
//    if (cp.isUpdatable()[m.periodic_pairs[i]])
      sys.periodicity(m.periodic_pairs[i+0],m.periodic_pairs[i+1]);


//sys.print("sys_prebc_bc_" + boost::lexical_cast<std::string>(m.irank) + ".plt");

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
  Common::PE::Comm::instance().barrier();

//  // filling the system with prescribed values
//  sys.reset();
//  vals=&m.mat_presolve[0];
//  for (int i=0; i<(const int)m.global_numbering.size(); i++)
//    if (cp.isUpdatable()[i])
//      for (int j=0; j<(const int)m.nbeqs; j++)
//        for (int k=m.rowstart_positions[i]; k<(const int)(m.rowstart_positions[i+1]); k++)
//          for (int l=0; l<(const int)m.nbeqs; l++)
//            sys.matrix()->set_value(m.column_indices[k]*m.nbeqs+l,i*m.nbeqs+j,*vals++);
//  vals=&m.rhs_presolve[0];
//  for (int i=0; i<(const int)m.global_numbering.size(); i++)
//    for (int j=0; j<(const int)m.nbeqs; j++)
//      sys.rhs()->set_value(i,j,*vals++);
//  vals=&m.sol_presolve[0];
//  for (int i=0; i<(const int)m.global_numbering.size(); i++)
//    for (int j=0; j<(const int)m.nbeqs; j++)
//      sys.solution()->set_value(i,j,*vals++);

//sys.print("sys_orig_" + boost::lexical_cast<std::string>(m.irank) + ".plt");

  // and solve the system
  sys.solve();

  // check results
  std::vector<Real> v;
  sys.solution()->data(v);
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    if (cp.isUpdatable()[i])
      for (int j=0; j<(const int)m.nbeqs; j++)
        BOOST_CHECK_CLOSE(v[i*m.nbeqs+j],m.result[i*m.nbeqs+j],1e-3);

//  sys.print("distributed_system_" + boost::lexical_cast<std::string>(m.irank) + ".plt");

  // print a test file
  std::ofstream fres(std::string("coords_with_sol_" + boost::lexical_cast<std::string>(m.irank) + ".plt").c_str());
  fres << "VARIABLES=\"X\",\"Y\",\"V0\",\"V1\",\"V2\"\nZONE T=\"Solve results of utest-lss-distributed-matrix.\"" << std::flush;
  fres.precision(15);
  sys.solution()->data(v);
  for (int i=0; i<(const int)m.global_numbering.size(); i++)
    if (cp.isUpdatable()[i])
      fres << m.nodal_coordinates[2*i+0] << " " << m.nodal_coordinates[2*i+1] << " " << v[m.nbeqs*i+0] << " " << v[m.nbeqs*i+1] << " " << v[m.nbeqs*i+2] << "\n" << std::flush;
  fres.close();

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_parallel_environment )
{
  CFinfo.setFilterRankZero(true);
  Common::PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL(Common::PE::Comm::instance().is_active(),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

