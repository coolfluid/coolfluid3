// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
  @file lss-trilinos Testbed for designing the linear solver interface and implementing trilinos behind.
  Will use Epetra::FEVbrMatrix for the time beign, TPetra does not support Stratimikos. Conversion seems reasonably simple according to trilinos doc.
**/

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/MPI/PE.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/System.hpp"

#include "Common/MPI/debug.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::Comm;
using namespace CF::Math;
using namespace CF::Math::LSS;


int main(void)
{
  // get to business
  PE::instance().init();
  int irank=PE::instance().rank();
  int nproc=PE::instance().size();

  // build commpattern
  CommPattern cp("commpattern");
  std::vector<Uint> gid;
  std::vector<Uint> rank_updatable;
  if (irank==0)
  {
    gid += 0,1,2,3,4;
    rank_updatable += 0,0,0,0,1;
  } else {
    gid += 3,4,5,6,7,8,9;
    rank_updatable += 0,1,1,1,1,1,1;
  }
  cp.insert("gid",gid,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<CommWrapper>(),rank_updatable);

  // build system
  std::vector<Uint> nodeconn;
  std::vector<Uint> startidxs;
  if (irank==0)
  {
    nodeconn += 0,1,0,1,2,1,2,3,2,3,4,3,4;
    startidxs += 0,2,5,8,11,13;
  } else {
    nodeconn += 0,1,0,1,2,1,2,3,2,3,4,3,4,5,4,5,6,5,6   ;
    startidxs +=  0,2,5,8,11,14,17,19;
  }
  System::Ptr sys(new System("sys"));
  sys->options().option("solver").change_value(lexical_cast<string>("Trilinos"));
  sys->create(cp,2,nodeconn,startidxs);

  // write a settings file for trilinos, solving with plain bicgstab, no preconditioning
  if (irank==0)
  {
    ofstream trilinos_xml("trilinos_settings.xml");
    trilinos_xml << "<ParameterList>\n";
    trilinos_xml << "  <Parameter name=\"Linear Solver Type\" type=\"string\" value=\"AztecOO\"/>\n";
    trilinos_xml << "  <ParameterList name=\"Linear Solver Types\">\n";
    trilinos_xml << "    <ParameterList name=\"AztecOO\">\n";
    trilinos_xml << "      <ParameterList name=\"Forward Solve\">\n";
    trilinos_xml << "        <ParameterList name=\"AztecOO Settings\">\n";
    trilinos_xml << "          <Parameter name=\"Aztec Solver\" type=\"string\" value=\"BiCGStab\"/>\n";
    trilinos_xml << "        </ParameterList>\n";
    trilinos_xml << "        <Parameter name=\"Max Iterations\" type=\"int\" value=\"400\"/>\n";
    trilinos_xml << "        <Parameter name=\"Tolerance\" type=\"double\" value=\"1e-13\"/>\n";
    trilinos_xml << "      </ParameterList>\n";
    trilinos_xml << "    </ParameterList>\n";
    trilinos_xml << "  </ParameterList>\n";
    trilinos_xml << "  <Parameter name=\"Preconditioner Type\" type=\"string\" value=\"None\"/>\n";
    trilinos_xml << "</ParameterList>\n";
    trilinos_xml.close();
  }

  // set intital values and boundary conditions
  sys->matrix()->reset(-0.5);
  sys->solution()->reset(1.);
  sys->rhs()->reset(0.);
  if (irank==0)
  {
    std::vector<Real> diag(10,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(0,0,1.);
    sys->dirichlet(0,1,1.);
  } else {
    std::vector<Real> diag(14,1.);
    sys->set_diagonal(diag);
    sys->dirichlet(6,0,10.);
    sys->dirichlet(6,1,10.);
  }

  // solve and print
  sys->solve();
  sys->print("test_system_" + boost::lexical_cast<std::string>(irank) + ".plt");
PEProcessSortedExecute(-1,
  sys->solution()->print(std::cout);
);

  // afscheid
  Comm::PE::instance().finalize();
};






/* SINGLE PROCESSOR CASE
int main(void)
{
  // get to business
  PE::instance().init();
  int irank=PE::instance().rank();
  int nproc=PE::instance().size();

  // build commpattern
  CommPattern cp("commpattern");
  std::vector<Uint> gid;
  std::vector<Uint> rank_updatable;
  gid += 0,1,2,3,4,5,6,7,8,9;
  rank_updatable += 0,0,0,0,0,0,0,0,0,0;
  cp.insert("gid",gid,1,false);
  cp.setup(cp.get_child_ptr("gid")->as_ptr<CommWrapper>(),rank_updatable);

  // build system
  std::vector<Uint> nodeconn;
  nodeconn += 0,1,0,1,2,1,2,3,2,3,4,3,4,5,4,5,6,5,6,7,6,7,8,7,8,9,8,9;
  std::vector<Uint> startidxs;
  startidxs += 0,2,5,8,11,14,17,20,23,26,28;
  System::Ptr sys(new System("sys"));
  sys->options().option("solver").change_value(lexical_cast<string>("Trilinos"));
  sys->create(cp,2,nodeconn,startidxs);

  // write a settings file for trilinos, solving with plain bicgstab, no preconditioning
  ofstream trilinos_xml("trilinos_settings.xml");
  trilinos_xml << "<ParameterList>\n";
  trilinos_xml << "  <Parameter name=\"Linear Solver Type\" type=\"string\" value=\"AztecOO\"/>\n";
  trilinos_xml << "  <ParameterList name=\"Linear Solver Types\">\n";
  trilinos_xml << "    <ParameterList name=\"AztecOO\">\n";
  trilinos_xml << "      <ParameterList name=\"Forward Solve\">\n";
  trilinos_xml << "        <ParameterList name=\"AztecOO Settings\">\n";
  trilinos_xml << "          <Parameter name=\"Aztec Solver\" type=\"string\" value=\"BiCGStab\"/>\n";
  trilinos_xml << "        </ParameterList>\n";
  trilinos_xml << "        <Parameter name=\"Max Iterations\" type=\"int\" value=\"400\"/>\n";
  trilinos_xml << "        <Parameter name=\"Tolerance\" type=\"double\" value=\"1e-13\"/>\n";
  trilinos_xml << "      </ParameterList>\n";
  trilinos_xml << "    </ParameterList>\n";
  trilinos_xml << "  </ParameterList>\n";
  trilinos_xml << "  <Parameter name=\"Preconditioner Type\" type=\"string\" value=\"None\"/>\n";
  trilinos_xml << "</ParameterList>\n";
  trilinos_xml.close();

  // set intital values
  sys->matrix()->reset(-0.5);
  sys->solution()->reset(1.);
  sys->rhs()->reset(0.);
  std::vector<Real> diag(20,1.);
  sys->set_diagonal(diag);
  sys->dirichlet(0,0,1.);
  sys->dirichlet(0,1,1.);
  sys->dirichlet(9,0,10.);
  sys->dirichlet(9,1,10.);

  // printing a reference matrix for matlab
  std::vector<Uint> rows;
  std::vector<Uint> cols;
  std::vector<Real> vals;
  RealMatrix m(20,20);
  m.setConstant(0.);
  sys->matrix()->data(rows,cols,vals);
PEProcessSortedExecute(-1,
  for (int i=0; i<vals.size(); i++) m(rows[i],cols[i])=vals[i];
  cout << "A =  [" << flush;
  for (int i=0; i<20; i++)
  {
    cout << m(i,0) << flush;
    for (int j=1; j<20; j++) cout << ", " << m(i,j) << flush;
    if (i!=19) cout << "; " << flush;
  }
  cout <<  "]\n" << flush;
  sys->rhs()->data(vals);
  cout << "b =  [" << vals[0] << flush;
  for (int i=1; i<vals.size(); i++) cout << "; " << vals[i] << flush;
  cout <<  "]\n" << flush;

);

  // solve and print
  sys->solve();
  sys->print("test_system_" + boost::lexical_cast<std::string>(irank) + ".plt");
PEProcessSortedExecute(-1,
  sys->solution()->print(std::cout);
);

  // afscheid
  Comm::PE::instance().finalize();
};
*/
