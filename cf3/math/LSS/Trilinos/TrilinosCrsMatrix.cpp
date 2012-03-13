// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <boost/pointer_cast.hpp>

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"
#include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Thyra_EpetraLinearOp.hpp"
#include "Epetra_MsrMatrix.h"
#include "Epetra_Vector.h"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_CommandLineProcessor.hpp"

#include "common/Assertions.hpp"
#include "common/Builder.hpp"
#include "common/PE/Comm.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosCrsMatrix.cpp implementation of LSS::TrilinosCrsMatrix
  @author Tamas Banyai

  It is based on Trilinos's FEVbrMatrix.
**/

////////////////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::LSS;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LSS::TrilinosCrsMatrix, LSS::Matrix, LSS::LibLSS > TrilinosCrsMatrix_Builder;

TrilinosCrsMatrix::TrilinosCrsMatrix(const std::string& name) :
  LSS::Matrix(name),
  m_mat(0),
  m_is_created(false),
  m_neq(0),
  m_num_my_elements(0),
  m_p2m(0),
  m_converted_indices(0),
  m_comm(common::PE::Comm::instance().communicator())
{
  properties().add_property("vector_type", std::string("cf3.math.LSS.TrilinosVector"));
  options().add_option( "settings_file", "trilinos_settings.xml" );
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::create(cf3::common::PE::CommPattern& cp, const Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector& solution, LSS::Vector& rhs)
{
  // if already created
  if (m_is_created) destroy();

  // get global ids vector
  int *gid=(int*)cp.gid()->pack();

  // prepare intermediate data
  int max_nb_row_entries=0;
  std::vector<int> num_indices_per_row;
  std::vector<int> my_global_elements;

  const Uint nb_nodes_for_rank = cp.isUpdatable().size();
  cf3_assert(nb_nodes_for_rank+1 == starting_indices.size());
  my_global_elements.reserve(nb_nodes_for_rank*neq);
  num_indices_per_row.reserve(nb_nodes_for_rank*neq);

  for (int i=0; i<nb_nodes_for_rank; i++)
  {
    if (cp.isUpdatable()[i])
    {
      m_num_my_elements += neq;
      const int start_gid = gid[i]*neq;
      for(int j = 0; j != neq; ++j)
        my_global_elements.push_back(start_gid+j);
      const int row_nb_values = static_cast<int>(neq*(starting_indices[i+1]-starting_indices[i]));
      num_indices_per_row.insert(num_indices_per_row.end(), neq, row_nb_values);
      max_nb_row_entries=max_nb_row_entries < row_nb_values ? row_nb_values : max_nb_row_entries;
    }
  }

  // process local to matrix local numbering mapper
  int iupd=0;
  int ighost=m_num_my_elements;
  m_p2m.resize(0);
  m_p2m.reserve(nb_nodes_for_rank*neq);
  for (int i=0; i<nb_nodes_for_rank; ++i)
  {
    if (cp.isUpdatable()[i])
    {
      for(Uint j = 0; j != neq; ++j)
        m_p2m.push_back(iupd++);
    }
    else
    {
      for(Uint j = 0; j != neq; ++j)
        m_p2m.push_back(ighost++);
    }
  }

  // rowmap, ghosts not present
  Epetra_Map rowmap(-1,m_num_my_elements,&my_global_elements[0],0,m_comm);

  // append the ghosts at the end of the element list
  for (int i=0; i<nb_nodes_for_rank; i++)
  {
    if (!cp.isUpdatable()[i])
    {
      const int start_gid = gid[i]*neq;
      for(int j = 0; j != neq; ++j)
        my_global_elements.push_back(start_gid+j);
    }
  }
  // colmap, has ghosts at the end
  Epetra_Map colmap(-1,nb_nodes_for_rank*neq,&my_global_elements[0],0,m_comm);
  my_global_elements.clear();

  // Create the graph, using static profile for performance
  Epetra_CrsGraph graph(Copy, rowmap, colmap, &num_indices_per_row[0], true);

  // Fill the graph
  m_converted_indices.resize(max_nb_row_entries);
  for(int i = 0; i != nb_nodes_for_rank; ++i)
  {
    if(cp.isUpdatable()[i])
    {
      const Uint columns_begin = starting_indices[i];
      const Uint columns_end = starting_indices[i+1];
      for(Uint j = columns_begin; j != columns_end; ++j)
      {
        const Uint column = j-columns_begin;
        const Uint node_idx = node_connectivity[j]*neq;
        for(int k = 0; k != neq; ++k)
        {
          m_converted_indices[column*neq+k] = m_p2m[node_idx+k];
        }
      }
      for(int k = 0; k != neq; ++k)
      {
        const int row = m_p2m[i*neq+k];
        TRILINOS_THROW(graph.InsertMyIndices(row, static_cast<int>(neq*(columns_end - columns_begin)), &m_converted_indices[0]));
      }
    }
  }

  TRILINOS_THROW(graph.FillComplete());
  TRILINOS_THROW(graph.OptimizeStorage());

  // create matrix
  m_mat=Teuchos::rcp(new Epetra_CrsMatrix(Copy, graph));
  TRILINOS_THROW(m_mat->FillComplete());
  TRILINOS_THROW(m_mat->OptimizeStorage());

  delete[] gid;

  // set class properties
  m_is_created=true;
  m_neq=neq;
  CFdebug << "Created a " << m_mat->NumGlobalCols() << " x " << m_mat->NumGlobalRows() << " trilinos matrix with " << m_mat->NumGlobalNonzeros() << " non-zero elements." << CFendl;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::destroy()
{
  if (m_is_created)
  {
    m_mat.reset();
  }
  m_p2m.resize(0);
  m_p2m.reserve(0);
  m_neq=0;
  m_num_my_elements=0;
  m_is_created=false;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::set_value(const Uint icol, const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  TRILINOS_THROW(m_mat->ReplaceMyValues(m_p2m[irow], 1, &value, &m_p2m[icol]));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::add_value(const Uint icol, const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  TRILINOS_THROW(m_mat->SumIntoMyValues(m_p2m[irow], 1, &value, &m_p2m[icol]));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::get_value(const Uint icol, const Uint irow, Real& value)
{
  cf3_assert(m_is_created);
  int num_entries;
  Real* extracted_values;
  int* extracted_indices;
  TRILINOS_THROW(m_mat->ExtractMyRowView(m_p2m[irow], num_entries, extracted_values, extracted_indices));
  for(int i = 0; i != num_entries; ++i)
  {
    if(extracted_indices[i] == m_p2m[icol])
    {
      value = extracted_values[i];
      return;
    }
  }
  throw common::BadValue(FromHere(),"Trying to access an illegal entry.");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::solve(LSS::Vector& solution, LSS::Vector& rhs)
{
  cf3_assert(m_is_created);
  cf3_assert(solution.is_created());
  cf3_assert(rhs.is_created());

  // general setup phase
  Stratimikos::DefaultLinearSolverBuilder linearSolverBuilder(options().option("settings_file").value_str());
  /// @todo decouple from fancyostream to ostream or to C stdout when possible
  Teuchos::RCP<Teuchos::FancyOStream> out = Teuchos::VerboseObjectBase::getDefaultOStream();
  Teuchos::CommandLineProcessor  clp(false); // false: don't throw exceptions
  linearSolverBuilder.setupCLP(&clp); // not used, TODO: see if can be removed safely since not really used
  /// @todo check whgats wrtong with input options via string
  //clp.setOption( "tol",            &tol,            "Tolerance to check against the scaled residual norm." ); // input options via string, not working for some reason
  int argc=0; char** argv=0; Teuchos::CommandLineProcessor::EParseCommandLineReturn parse_return = clp.parse(argc,argv);
  if( parse_return != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL ) throw common::ParsingFailed(FromHere(),"Emulated command line parsing for stratimikos failed");

  // wrapping epetra to thyra
  Teuchos::RCP<const Thyra::LinearOpBase<double> > A = Thyra::epetraLinearOp( m_mat );
  LSS::TrilinosVector& tsol = dynamic_cast<LSS::TrilinosVector&>(solution);
  Teuchos::RCP<Thyra::VectorBase<double> >         x = Thyra::create_Vector( tsol.epetra_vector() , A->domain() );
  LSS::TrilinosVector& trhs = dynamic_cast<LSS::TrilinosVector&>(rhs);
  Teuchos::RCP<const Thyra::VectorBase<double> >   b = Thyra::create_Vector( trhs.epetra_vector() , A->range() );

  // r = b - A*x, initial L2 norm
  double norm2_in=0.;
  {
    Epetra_Vector epetra_r(*trhs.epetra_vector());
    Epetra_Vector m_mat_x(m_mat->OperatorRangeMap());
    m_mat->Apply(*tsol.epetra_vector(),m_mat_x);
    epetra_r.Update(-1.0,m_mat_x,1.0);
    epetra_r.Norm2(&norm2_in);
  }

  // Reading in the solver parameters from the parameters file and/or from
  // the command line.  This was setup by the command-line options
  // set by the setupCLP(...) function above.
  linearSolverBuilder.readParameters(0); // out.get() if want confirmation about the xml file within trilinos
  Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> > lowsFactory = linearSolverBuilder.createLinearSolveStrategy(""); // create linear solver strategy
/// @todo verbosity level from option
  lowsFactory->setVerbLevel((Teuchos::EVerbosityLevel)1); // set verbosity

  // print back default and current settings
  if (false) {
    std::ofstream ofs("./trilinos_default.txt");
    linearSolverBuilder.getValidParameters()->print(ofs,Teuchos::ParameterList::PrintOptions().indent(2).showTypes(true).showDoc(true)); // the last true-false is the deciding about whether printing documentation to option or not
    ofs.flush();ofs.close();
    ofs.open("./trilinos_default.xml");
    Teuchos::writeParameterListToXmlOStream(*linearSolverBuilder.getValidParameters(),ofs);
    ofs.flush();ofs.close();
  }
  if (false) {
    linearSolverBuilder.writeParamsFile(*lowsFactory,"./trilinos_current.xml");
  }

  // solve the matrix
  Teuchos::RCP<Thyra::LinearOpWithSolveBase<double> > lows = Thyra::linearOpWithSolve(*lowsFactory, A);
  lows->solve(Thyra::NOTRANS,*b,x.ptr());

  // r = b - A*x, final L2 norm
  double norm2_out=0.;
  {
    Epetra_Vector epetra_r(*trhs.epetra_vector());
    Epetra_Vector m_mat_x(m_mat->OperatorRangeMap());
    m_mat->Apply(*tsol.epetra_vector(),m_mat_x);
    epetra_r.Update(-1.0,m_mat_x,1.0);
    epetra_r.Norm2(&norm2_out);
  }

  // print in and out residuals
  CFinfo << "Solver residuals: in " << norm2_in << ", out " << norm2_out << CFendl;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::set_values(const BlockAccumulator& values)
{
  cf3_assert(m_is_created);
  const Uint nb_nodes = values.indices.size();
  const int num_entries = nb_nodes*m_neq;
  cf3_assert(values.mat.rows() == num_entries);
  // Convert the index vector
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    const Uint local_start_idx = values.indices[i]*m_neq;
    for(int j = 0; j != m_neq; ++j)
      m_converted_indices[i*m_neq+j] = m_p2m[local_start_idx+j];
  }
  // insert the values
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    for(int j = 0; j != m_neq; ++j)
    {
      if(m_converted_indices[i*m_neq+j] < m_num_my_elements)
        TRILINOS_THROW(m_mat->ReplaceMyValues(m_converted_indices[i*m_neq+j], num_entries, values.mat.data()+(num_entries*(i*m_neq+j)),&m_converted_indices[0]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::add_values(const BlockAccumulator& values)
{
  cf3_assert(m_is_created);
  const Uint nb_nodes = values.indices.size();
  const int num_entries = nb_nodes*m_neq;
  cf3_assert(values.mat.rows() == num_entries);
  // Convert the index vector
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    const Uint local_start_idx = values.indices[i]*m_neq;
    for(int j = 0; j != m_neq; ++j)
      m_converted_indices[i*m_neq+j] = m_p2m[local_start_idx+j];
  }
  // insert the values
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    for(int j = 0; j != m_neq; ++j)
    {
      if(m_converted_indices[i*m_neq+j] < m_num_my_elements)
        TRILINOS_THROW(m_mat->SumIntoMyValues(m_converted_indices[i*m_neq+j], num_entries, values.mat.data()+(num_entries*(i*m_neq+j)),&m_converted_indices[0]));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::get_values(BlockAccumulator& values)
{
  cf3_assert(m_is_created);
  values.mat.setZero();
  const Uint nb_nodes = values.indices.size();
  const int num_entries = nb_nodes*m_neq;
  int extracted_num_entries;
  Real* extracted_values;
  int* extracted_indices;
  cf3_assert(values.mat.rows() == num_entries);
  std::map<int, int> reverse_idx_map;
  // Convert the index vector
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    const Uint local_start_idx = values.indices[i]*m_neq;
    for(int j = 0; j != m_neq; ++j)
    {
      m_converted_indices[i*m_neq+j] = m_p2m[local_start_idx+j];
      reverse_idx_map[m_p2m[local_start_idx+j]] = i*m_neq + j;
    }
  }
  // get the values
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    for(int j = 0; j != m_neq; ++j)
    {
      if(m_converted_indices[i*m_neq+j] >= m_num_my_elements)
        continue;
      TRILINOS_THROW(m_mat->ExtractMyRowView(m_converted_indices[i*m_neq+j], extracted_num_entries, extracted_values, extracted_indices));
      for(int k = 0; k != extracted_num_entries; ++k)
      {
        const std::map<int,int>::const_iterator it = reverse_idx_map.find(extracted_indices[k]);
        if(it != reverse_idx_map.end())
          values.mat(i*m_neq+j, it->second) = extracted_values[k];
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval)
{
  cf3_assert(m_is_created);
  int num_entries;
  Real* extracted_values;
  int* extracted_indices;
  const int row = iblockrow*m_neq+ieq;

  if(m_p2m[row] >= m_num_my_elements)
    return;

  TRILINOS_THROW(m_mat->ExtractMyRowView(m_p2m[row], num_entries, extracted_values, extracted_indices));
  for(int i = 0; i != num_entries; ++i)
  {
    if(extracted_indices[i] == m_p2m[row])
      extracted_values[i] = diagval;
    else
      extracted_values[i] = offdiagval;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values)
{
  throw common::NotImplemented(FromHere(), "get_column_and_replace_to_zero is not implemented for TrilinosCrsMatrix");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf3_assert(m_is_created);
  int num_entries_from;
  Real* extracted_values_from;
  int* extracted_indices_from;
  const int row_from_begin = iblockrow_from*m_neq;

  int num_entries_to;
  Real* extracted_values_to;
  int* extracted_indices_to;
  const int row_to_begin = iblockrow_to*m_neq;

  if(m_p2m[row_from_begin] >= m_num_my_elements || m_p2m[row_to_begin] >= m_num_my_elements)
    return;

  const int nb_eq = m_neq;
  for(int i = 0; i != nb_eq; ++i)
  {
    TRILINOS_THROW(m_mat->ExtractMyRowView(m_p2m[row_from_begin+i], num_entries_from, extracted_values_from, extracted_indices_from));
    TRILINOS_THROW(m_mat->ExtractMyRowView(m_p2m[row_to_begin+i], num_entries_to, extracted_values_to, extracted_indices_to));
    if (num_entries_to != num_entries_from)
    {
      throw common::BadValue(FromHere(),"Number of entries do not match for the two block rows to be tied together.");
    }
    int diag, pair;
    for(int j = 0; j != num_entries_from; ++j)
    {
      if(extracted_indices_from[j] != extracted_indices_to[j])
      {
        throw common::BadValue(FromHere(),"Indices of the entries do not match for the two block rows to be tied together.");
      }

      if(extracted_indices_from[j] == m_p2m[row_from_begin+i])
        diag = j;
      if(extracted_indices_to[j] == m_p2m[row_to_begin+i])
        pair = j;

      extracted_values_to[j] += extracted_values_from[j];
      extracted_values_from[j] = 0.;
    }
    extracted_values_from[diag] = 1.;
    extracted_values_from[pair] = -1.;
    for(int k = 0; k != nb_eq; ++k)
    {
      extracted_values_to[pair-i+k] += extracted_values_to[diag-i+k];
      extracted_values_to[diag-i+k] = 0.;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::set_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  cf3_assert(diag.size() == m_p2m.size());
  Epetra_Vector new_diag(m_mat->RowMap());
  new_diag.ReplaceMyValues(m_p2m.size(), &diag[0], &m_p2m[0]);
  m_mat->ReplaceDiagonalValues(new_diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::add_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  cf3_assert(diag.size() == m_p2m.size());
  Epetra_Vector new_diag(m_mat->RowMap());
  m_mat->ExtractDiagonalCopy(new_diag);
  new_diag.SumIntoMyValues(m_p2m.size(), &diag[0], &m_p2m[0]);
  m_mat->ReplaceDiagonalValues(new_diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::get_diagonal(std::vector<Real>& diag)
{
  cf3_assert(m_is_created);
  const int nb_col_entries = m_p2m.size();
  diag.resize(nb_col_entries);
  Epetra_Vector output_diag(m_mat->RowMap());
  m_mat->ExtractDiagonalCopy(output_diag);
  double* diag_view;
  output_diag.ExtractView(&diag_view);
  for(Uint i = 0; i != nb_col_entries; ++i)
  {
    diag[i] = m_p2m[i] < m_num_my_elements ? diag_view[m_p2m[i]] : 0.;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::reset(Real reset_to)
{
  cf3_assert(m_is_created);
  CFdebug << "Resetting CrsMatrix to " << reset_to << CFendl;
  TRILINOS_THROW(m_mat->PutScalar(reset_to));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::print(common::LogStream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int num_entries;
    Real* extracted_values;
    int* extracted_indices;

    const int nb_elems = m_p2m.size();
    std::vector<int> m2p(nb_elems);
    for (int i = 0; i != nb_elems; ++i)
      m2p[m_p2m[i]]=i;

    const int nb_rows = m_num_my_elements;
    for(int row = 0; row != nb_rows; ++row)
    {
      TRILINOS_THROW(m_mat->ExtractMyRowView(row, num_entries, extracted_values, extracted_indices));
      for(int i = 0; i != num_entries; ++i)
      {
        stream << row << " " << -m2p[extracted_indices[i]] << " " << extracted_values[i] << CFendl;
      }
      sumentries += num_entries;
    }
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_num_my_elements << "\n";
    stream << "# number of cols:       " << m_p2m.size() << "\n";
    stream << "# number of block rows: " << m_num_my_elements/neq() << "\n";
    stream << "# number of block cols: " << m_p2m.size()/neq() << "\n";
    stream << "# number of entries:    " << sumentries << "\n";
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::print(std::ostream& stream)
{
  if (m_is_created)
  {
    int sumentries=0;
    int num_entries;
    Real* extracted_values;
    int* extracted_indices;

    const int nb_elems = m_p2m.size();
    std::vector<int> m2p(nb_elems);
    for (int i = 0; i != nb_elems; ++i)
      m2p[m_p2m[i]]=i;

    const int nb_rows = m_num_my_elements;
    for(int row = 0; row != nb_rows; ++row)
    {
      TRILINOS_THROW(m_mat->ExtractMyRowView(row, num_entries, extracted_values, extracted_indices));
      for(int i = 0; i != num_entries; ++i)
      {
        stream << m2p[row] << " " << -m2p[extracted_indices[i]] << " " << extracted_values[i] << std::endl;
      }
      sumentries += num_entries;
    }
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_num_my_elements << "\n";
    stream << "# number of cols:       " << m_p2m.size() << "\n";
    stream << "# number of block rows: " << m_num_my_elements/neq() << "\n";
    stream << "# number of block cols: " << m_p2m.size()/neq() << "\n";
    stream << "# number of entries:    " << sumentries << "\n";
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::print(const std::string& filename, std::ios_base::openmode mode )
{
  std::ofstream stream(filename.c_str(),mode);
  stream << "VARIABLES=COL,ROW,VAL\n" << std::flush;
  stream << "ZONE T=\"" << type_name() << "::" << name() <<  "\"\n" << std::flush;
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::debug_data(std::vector<Uint>& row_indices, std::vector<Uint>& col_indices, std::vector<Real>& values)
{
  row_indices.clear(); col_indices.clear(); values.clear();
  const int nnz = m_mat->NumMyNonzeros();
  row_indices.reserve(nnz); col_indices.reserve(nnz); values.reserve(nnz);
  Real* extracted_values;
  int* extracted_indices;
  int num_entries;

  const int nb_elems = m_p2m.size();
  std::vector<int> m2p(nb_elems);
  for (int i = 0; i != nb_elems; ++i)
    m2p[m_p2m[i]]=i;

  const int nb_rows = m_num_my_elements;
  for(int row = 0; row != nb_rows; ++row)
  {
    TRILINOS_THROW(m_mat->ExtractMyRowView(row, num_entries, extracted_values, extracted_indices));
    for(int i = 0; i != num_entries; ++i)
    {
      row_indices.push_back(m2p[row]);
      col_indices.push_back(m2p[extracted_indices[i]]);
      values.push_back(extracted_values[i]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
