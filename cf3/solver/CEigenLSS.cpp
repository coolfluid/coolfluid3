// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>

#include "coolfluid-packages.hpp"

#ifdef CF3_HAVE_TRILINOS
  #include <Epetra_SerialComm.h>
  #include <Epetra_Map.h>
  #include <Epetra_Vector.h>
  #include <Epetra_CrsMatrix.h>

  #include "Stratimikos_DefaultLinearSolverBuilder.hpp"
  #include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"
  #include "Thyra_EpetraThyraWrappers.hpp"
  #include "Thyra_EpetraLinearOp.hpp"
//  #include "Teuchos_GlobalMPISession.hpp"
  #include "Teuchos_VerboseObject.hpp"
  #include "Teuchos_XMLParameterListHelpers.hpp"
  #include "Teuchos_CommandLineProcessor.hpp"

#else
  #ifdef CF3_HAVE_SUPERLU
    #define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
    #include <Eigen/SuperLUSupport>
  #endif
#endif

#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionURI.hpp"
#include "common/PE/Comm.hpp"
#include "common/Timer.hpp"

#include "mesh/Field.hpp"

#include "CEigenLSS.hpp"


namespace cf3 {
namespace solver {

using namespace cf3::common;
using namespace cf3::mesh;

cf3::common::ComponentBuilder < CEigenLSS, common::Component, LibSolver > aCeigenLSS_Builder;

CEigenLSS::CEigenLSS ( const std::string& name ) : Component ( name )
{
  options().add_option< OptionURI >("config_file", URI())
      ->description("Solver config file")
      ->pretty_name("Config File")
      ->mark_basic()
      ->cast_to<OptionURI>()->supported_protocol(URI::Scheme::FILE);

  if(!PE::Comm::instance().is_active())
    PE::Comm::instance().init();
}

void CEigenLSS::set_config_file(const URI& path)
{
  configure_option("config_file", path);
}


void CEigenLSS::resize ( Uint nb_dofs )
{
  if(nb_dofs == (Uint) m_system_matrix.rows())
    return;

  m_system_matrix.resize(nb_dofs, nb_dofs);
  m_rhs.resize(nb_dofs);
  m_solution.resize(nb_dofs);

  set_zero();
}

Uint CEigenLSS::size() const
{
  return m_system_matrix.cols();
}

Real& CEigenLSS::at(const cf3::Uint row, const cf3::Uint col)
{
  return m_system_matrix.coeffRef(row, col);
}


void CEigenLSS::set_zero()
{
  m_system_matrix.setZero();
  m_rhs.setZero();
  m_solution.setZero();
}

void CEigenLSS::set_dirichlet_bc(const cf3::Uint row, const cf3::Real value, const cf3::Real coeff)
{
  for(MatrixT::InnerIterator it(m_system_matrix, static_cast<int>(row)); it; ++it)
  {
    if(static_cast<Uint>(it.col()) != row)
    {
      it.valueRef() = 0.;
    }
    else
    {
      it.valueRef() = coeff;
    }
  }
  m_rhs[row] = coeff * value;
}

// This version kept symetry, but it is horribly inefficient because of the way we store the matrix
// void CEigenLSS::set_dirichlet_bc(const cf3::Uint row, const cf3::Real value, const cf3::Real coeff)
// {
//   for(int k=0; k < m_system_matrix.outerSize(); ++k)
//   {
//     for(MatrixT::InnerIterator it(m_system_matrix, k); it; ++it)
//     {
//       if((Uint) it.row() == row && (Uint) it.col() != row)
//       {
//         it.valueRef() = 0.;
//       }
//       else if((Uint) it.row() == row && (Uint) it.col() == row)
//       {
//         it.valueRef() = coeff;
//       }
//       else if((Uint)it.row() != row && (Uint) it.col() == row)
//       {
//         m_rhs[it.row()] -= it.value() * value;
//         it.valueRef() = 0.;
//       }
//     }
//   }
//
//   m_rhs[row] = coeff * value;
// }


RealVector& CEigenLSS::rhs()
{
  return m_rhs;
}

const RealVector& CEigenLSS::solution()
{
  return m_solution;
}


void CEigenLSS::solve()
{
#ifdef CF3_HAVE_TRILINOS
  Timer timer;
  const Uint nb_rows = size();
  cf3_assert(nb_rows == m_system_matrix.outerSize());

  Epetra_SerialComm comm;
  Epetra_Map map(nb_rows, 0, comm);

  Epetra_Vector ep_rhs(View, map, m_rhs.data());
  Epetra_Vector ep_sol(View, map, m_solution.data());

  // Count non-zeros
  std::vector<int> nnz(nb_rows, 0);
  for(int row=0; row < nb_rows; ++row)
  {
    for(MatrixT::InnerIterator it(m_system_matrix, row); it; ++it)
    {
      ++nnz[row];
    }
    cf3_assert(nnz[row]);
  }

  Epetra_CrsMatrix ep_A(Copy, map, &nnz[0]);
  time_matrix_construction = timer.elapsed(); timer.restart();

  // Fill the matrix
  for(int row=0; row < nb_rows; ++row)
  {
    std::vector<int> indices; indices.reserve(nnz[row]);
    std::vector<Real> values; values.reserve(nnz[row]);
    for(MatrixT::InnerIterator it(m_system_matrix, row); it; ++it)
    {
      indices.push_back(it.col());
      values.push_back(it.value());
    }
    ep_A.InsertGlobalValues(row, nnz[row], &values[0], &indices[0]);
  }

  ep_A.FillComplete();

  time_matrix_fill = timer.elapsed(); timer.restart();

///////////////////////////////////////////////////////////////////////////////////////////////
//BEGIN////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

  Teuchos::RCP<Epetra_CrsMatrix> epetra_A=Teuchos::rcpFromRef(ep_A);
  Teuchos::RCP<Epetra_Vector>    epetra_x=Teuchos::rcpFromRef(ep_sol);
  Teuchos::RCP<Epetra_Vector>    epetra_b=Teuchos::rcpFromRef(ep_rhs);

  const URI config_uri = option("config_file").value<URI>();
  const std::string config_path = config_uri.path();

  Stratimikos::DefaultLinearSolverBuilder linearSolverBuilder(config_path); // the most important in general setup

  Teuchos::RCP<Teuchos::FancyOStream> out = Teuchos::VerboseObjectBase::getDefaultOStream(); // TODO: decouple from fancyostream to ostream or to C stdout when possible
  typedef Teuchos::ParameterList::PrintOptions PLPrintOptions;
  Teuchos::CommandLineProcessor  clp(false); // false: don't throw exceptions

  Teuchos::RCP<const Thyra::LinearOpBase<double> > A = Thyra::epetraLinearOp( epetra_A );
  Teuchos::RCP<Thyra::VectorBase<double> >         x = Thyra::create_Vector( epetra_x, A->domain() );
  Teuchos::RCP<const Thyra::VectorBase<double> >   b = Thyra::create_Vector( epetra_b, A->range() );

  // r = b - A*x, initial L2 norm
  double nrm_r=0.;
  Real systemResidual=-1.;
  {
    Epetra_Vector epetra_r(*epetra_b);
    Epetra_Vector epetra_A_x(epetra_A->OperatorRangeMap());
    epetra_A->Apply(*epetra_x,epetra_A_x);
    epetra_r.Update(-1.0,epetra_A_x,1.0);
    epetra_r.Norm2(&nrm_r);
  }

  // Reading in the solver parameters from the parameters file and/or from
  // the command line.  This was setup by the command-line options
  // set by the setupCLP(...) function above.
  linearSolverBuilder.readParameters(0); // out.get() if want confirmation about the xml file within trilinos
  Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> > lowsFactory = linearSolverBuilder.createLinearSolveStrategy(""); // create linear solver strategy
  lowsFactory->setVerbLevel(Teuchos::VERB_NONE); // set verbosity

//  // print back default and current settings
//  if (opts->trilinos.dumpDefault!=0) {
//    fflush(stdout); cout << flush;
//    _MMESSAGE_(0,1,"Dumping Trilinos/Stratimikos solver defaults to files: 'trilinos_default.txt' and 'trilinos_default.xml'...\n");
//    fflush(stdout); cout << flush;
//    std::ofstream ofs("./trilinos_default.txt");
//    linearSolverBuilder.getValidParameters()->print(ofs,PLPrintOptions().indent(2).showTypes(true).showDoc(true)); // the last true-false is the deciding about whether printing documentation to option or not
//    ofs.flush();ofs.close();
//    ofs.open("./trilinos_default.xml");
//    Teuchos::writeParameterListToXmlOStream(*linearSolverBuilder.getValidParameters(),ofs);
//    ofs.flush();ofs.close();
//  }
//  if (opts->trilinos.dumpCurrXML!=0) {
//    fflush(stdout); cout << flush;
//    _MMESSAGE_(0,1,"Dumping Trilinos/Stratimikos current settings to file: 'trilinos_current.xml'...\n");
//    fflush(stdout); cout << flush;
//    linearSolverBuilder.writeParamsFile(*lowsFactory,"./trilinos_current.xml");
//  }

  time_solver_setup = timer.elapsed(); timer.restart();

  // solve the matrix
  Teuchos::RCP<Thyra::LinearOpWithSolveBase<double> > lows = Thyra::linearOpWithSolve(*lowsFactory, A); // create solver
  lows->solve(Thyra::NOTRANS, *b, x.ptr()); // solve

  time_solve = timer.elapsed(); timer.restart();

  // r = b - A*x, final L2 norm
  {
    Epetra_Vector epetra_r(*epetra_b);
    Epetra_Vector epetra_A_x(epetra_A->OperatorRangeMap());
    epetra_A->Apply(*epetra_x,epetra_A_x);
    epetra_r.Update(-1.0,epetra_A_x,1.0);
    systemResidual=1./nrm_r;
    nrm_r=0.;
    epetra_r.Norm2(&nrm_r);
    systemResidual*=nrm_r;
  }

  time_residual = timer.elapsed();

///////////////////////////////////////////////////////////////////////////////////////////////
//END//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

#else // no trilinos

#ifdef CF3_HAVE_SUPERLU
  Eigen::SparseMatrix<Real> A(m_system_matrix);
  Eigen::SparseLU<Eigen::SparseMatrix<Real>,Eigen::SuperLU> lu_of_A(A);
  if(!lu_of_A.solve(rhs(), &m_solution))
    throw common::FailedToConverge(FromHere(), "Solution failed.");
#else // no trilinos and no superlu
  RealMatrix A(m_system_matrix);
  Eigen::FullPivLU<RealMatrix> lu_of_A(A);
  m_solution = lu_of_A.solve(m_rhs);
#endif // end ifdef superlu

#endif // end ifdef trilinos

}

void CEigenLSS::print_matrix()
{
  std::cout << m_system_matrix << std::endl;
}


void increment_solution(const RealVector& solution, const std::vector<std::string>& field_names, const std::vector<std::string>& var_names, const std::vector<Uint>& var_sizes, Mesh& solution_mesh)
{
  const Uint nb_vars = var_names.size();

  std::vector<Uint> var_offsets;
  var_offsets.push_back(0);
  for(Uint i = 0; i != nb_vars; ++i)
  {
    var_offsets.push_back(var_offsets.back() + var_sizes[i]);
  }

  // Copy the data to the fields, where each field value is incremented with the value from the solution vector
  std::set<std::string> unique_field_names;
  boost_foreach(const std::string& field_name, field_names)
  {
    if(unique_field_names.insert(field_name).second)
    {
      Field& field = find_component_with_name<Field>(solution_mesh,field_name);
      const Uint field_size = field.size();
      for(Uint row_idx = 0; row_idx != field_size; ++row_idx)
      {
        Field::Row row = field[row_idx];
        for(Uint i = 0; i != nb_vars; ++i)
        {
          if(field_names[i] != field_name)
            continue;

          const Uint solution_begin = var_offsets.back() * row_idx + var_offsets[i];
          const Uint solution_end = solution_begin + var_sizes[i];
          Uint field_idx = field.var_index(var_names[i]);

          cf3_assert( (Uint) field.var_length(var_names[i]) == (Uint) var_sizes[i]);

          for(Uint sol_idx = solution_begin; sol_idx != solution_end; ++sol_idx)
          {
            row[field_idx++] += solution[sol_idx];
          }
        }
      }
    }
  }
}


} // solver
} // cf3
