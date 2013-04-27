// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>

#include <boost/pointer_cast.hpp>

#include "Teuchos_ConfigDefs.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include "Epetra_CrsGraph.h"
#include "Epetra_Vector.h"
#include "Epetra_LinearProblem.h"

// EpetraExt includes
#include "EpetraExt_CrsMatrixIn.h"
#include "EpetraExt_VectorIn.h"

#include "Thyra_EpetraLinearOp.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Thyra_LinearOpWithSolveBase.hpp"
#include "Thyra_VectorBase.hpp"

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"

#include "common/Assertions.hpp"
#include "common/Builder.hpp"
#include "common/PE/Comm.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TrilinosDetail.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"
#include "math/VariablesDescriptor.hpp"

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
  properties().add("vector_type", std::string("cf3.math.LSS.TrilinosVector"));
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::create(cf3::common::PE::CommPattern& cp, const Uint neq, const std::vector<Uint>& node_connectivity, const std::vector<Uint>& starting_indices, LSS::Vector& solution, LSS::Vector& rhs, const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active)
{
  boost::shared_ptr<VariablesDescriptor> single_var_descriptor = common::allocate_component<VariablesDescriptor>("SingleVariableDescriptor");
  single_var_descriptor->options().set(common::Tags::dimension(), neq);
  single_var_descriptor->push_back("LSSvars", VariablesDescriptor::Dimensionalities::VECTOR);
  create_blocked(cp, *single_var_descriptor, node_connectivity, starting_indices, solution, rhs, periodic_links_nodes, periodic_links_active);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, const std::vector< Uint >& node_connectivity, const std::vector< Uint >& starting_indices, Vector& solution, Vector& rhs, const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active)
{
  // if already created
  if (m_is_created) destroy();

  // Copy node connectivity
  m_node_connectivity.resize(node_connectivity.size());
  m_starting_indices.resize(starting_indices.size());
  std::copy(node_connectivity.begin(), node_connectivity.end(), m_node_connectivity.begin());
  std::copy(starting_indices.begin(), starting_indices.end(), m_starting_indices.begin());

  const Uint total_nb_eq = vars.size();

  // prepare intermediate data
  std::vector<int> my_global_elements;
  std::vector<Uint> my_ranks;

  create_map_data(cp, vars, m_p2m, my_global_elements, my_ranks, m_num_my_elements, periodic_links_nodes, periodic_links_active);
  std::vector<int> num_indices_per_row; num_indices_per_row.reserve(m_num_my_elements);
  std::vector<int> indices_per_row;
  create_indices_per_row(cp, vars, node_connectivity, starting_indices, m_p2m, num_indices_per_row, indices_per_row, periodic_links_nodes, periodic_links_active);

  m_converted_indices.resize(*std::max_element(num_indices_per_row.begin(), num_indices_per_row.end()));

  // rowmap, ghosts not present
  Epetra_Map rowmap(-1,m_num_my_elements,&my_global_elements[0],0,m_comm);

  // colmap, has ghosts at the end
  Epetra_Map colmap(-1,my_global_elements.size(),&my_global_elements[0],0,m_comm);
  my_global_elements.clear(); // no longer needed

  // Create the graph, using static profile for performance
  Epetra_CrsGraph graph(Copy, rowmap, colmap, &num_indices_per_row[0], true);

  // Fill the graph
  int row_start = 0;
  cf3_assert(num_indices_per_row.size() == m_num_my_elements);
  for(int i = 0; i != m_num_my_elements; ++i)
  {
    const int row_nb_elems = num_indices_per_row[i];
    cf3_assert( (row_start + row_nb_elems) <= indices_per_row.size() );
    TRILINOS_THROW(graph.InsertMyIndices(i, row_nb_elems, &indices_per_row[row_start]));
    row_start += row_nb_elems;
  }

  TRILINOS_THROW(graph.FillComplete());
  TRILINOS_THROW(graph.OptimizeStorage());

  // create matrix
  m_mat=Teuchos::rcp(new Epetra_CrsMatrix(Copy, graph));
  TRILINOS_THROW(m_mat->FillComplete());
  TRILINOS_THROW(m_mat->OptimizeStorage());

  // set class properties
  m_is_created=true;
  m_neq=total_nb_eq;
  CFdebug << "Rank " << common::PE::Comm::instance().rank() << ": Created a " << m_mat->NumGlobalCols() << " x " << m_mat->NumGlobalRows() << " trilinos matrix with " << m_mat->NumGlobalNonzeros() << " non-zero elements and " << m_num_my_elements << " local rows" << CFendl;
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

void TrilinosCrsMatrix::symmetric_dirichlet(const Uint blockrow, const Uint ieq, const Real value, Vector& rhs)
{
  // We assume that we have an epetra RHS with the same storage structure as the matrix!
  Epetra_Vector& epetra_rhs = *dynamic_cast<TrilinosVector&>(rhs).epetra_vector();

  int num_entries;
  Real* extracted_values;
  int* extracted_indices;

  const int bc_col = m_p2m[blockrow*m_neq+ieq];

  m_dirichlet_nodes.push_back(std::make_pair(blockrow, ieq));

  const Uint nb_connected_nodes = m_starting_indices[blockrow+1] - m_starting_indices[blockrow];
  std::vector<int> row_indices; row_indices.reserve(m_neq*nb_connected_nodes);
  const Uint conn_start = m_starting_indices[blockrow];
  const Uint conn_end = m_starting_indices[blockrow+1];
  for(Uint i = conn_start; i != conn_end; ++i)
  {
    for(Uint j = 0; j != m_neq; ++j)
    {
      row_indices.push_back(m_p2m[m_node_connectivity[i]*m_neq+j]);
    }
  }
  
  DirichletEntryT& cached_col_values = m_symmetric_dirichlet_values[bc_col];

  if(cached_col_values.empty())
  {
    BOOST_FOREACH(const int other_row, row_indices)
    {
      if(other_row >= m_num_my_elements)
        continue;

      TRILINOS_THROW(m_mat->ExtractMyRowView(other_row, num_entries, extracted_values, extracted_indices));
      const int nb_entries_const = num_entries;
      Uint i;
      if(other_row != bc_col)
      {
        for(i = 0; i != nb_entries_const; )
        {
          if(extracted_indices[i] == bc_col)
          {
            cached_col_values[other_row] = extracted_values[i];
            epetra_rhs[other_row] -= extracted_values[i] * value;
            extracted_values[i] = 0;
            break;
          }
          ++i;
        }
        cf3_assert(i != nb_entries_const);
      }
      else
      {
        for(i = 0; i != nb_entries_const; ++i)
        {
          if(extracted_indices[i] == bc_col)
          {
            extracted_values[i] = 1.;
          }
          else
          {
            extracted_values[i] = 0;
          }
        }
      }
    }
  }
  else // Reuse the cached values, if the matrix wasn't reset since the previous BC application
  {
    for(DirichletEntryT::const_iterator it = cached_col_values.begin(); it != cached_col_values.end(); ++it)
    {
      epetra_rhs[it->first] -= it->second * value;
    }
  }

  rhs.set_value(blockrow, ieq, value);
}

////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<std::pair<Uint, Uint> >& TrilinosCrsMatrix::get_dirichlet_nodes() const
{
  return m_dirichlet_nodes;
}

///////////////////////////////////////////////////////////////////////////////////////////////

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

  m_symmetric_dirichlet_values.clear();
  m_dirichlet_nodes.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::clone_to(Matrix &other)
{
  if(!m_is_created)
    throw common::SetupError(FromHere(), "Matrix to clone " + uri().string() + " is not created");

  TrilinosCrsMatrix* other_ptr = dynamic_cast<TrilinosCrsMatrix*>(&other);
  if(is_null(other_ptr))
    throw common::SetupError(FromHere(), "clone_to method of TrilinosCrsMatrix needs another TrilinosCrsMatrix, but a " + other.derived_type_name() + " was supplied instead.");

  other_ptr->m_mat = Teuchos::rcp(new Epetra_CrsMatrix(*m_mat));
  other_ptr->m_is_created = m_is_created;
  other_ptr->m_neq = m_neq;
  other_ptr->m_num_my_elements = m_num_my_elements;
  other_ptr->m_p2m = m_p2m;
  other_ptr->m_converted_indices = m_converted_indices;
  other_ptr->m_node_connectivity = m_node_connectivity;
  other_ptr->m_starting_indices = m_starting_indices;
  other_ptr->m_symmetric_dirichlet_values = m_symmetric_dirichlet_values;
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
        stream << m2p[extracted_indices[i]] << " " << -m2p[row] << " " << extracted_values[i] << std::endl;
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

void TrilinosCrsMatrix::print_native(ostream& stream)
{
  m_mat->Print(stream);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::blocked_var_gids ( const VariablesDescriptor& var_descriptor, std::vector< std::vector< int > >& var_gids )
{
  cf3_assert(var_descriptor.size() == m_neq);
  cf3_assert(m_p2m.size() % m_neq == 0);
  const Uint nb_block_rows = m_p2m.size() / m_neq;
  
  const Epetra_Map & range_map = m_mat->OperatorRangeMap();
  cf3_assert(range_map.NumMyElements() == m_num_my_elements);
  int * matrix_global_ids = range_map.MyGlobalElements();
  
  const Uint nb_vars = var_descriptor.nb_vars();
  std::vector< std::set<int> > var_gid_sets(nb_vars);
  
  for(Uint i = 0; i != nb_block_rows; ++i)
  {
    for(Uint j = 0; j != nb_vars; ++j)
    {
      const Uint var_begin = var_descriptor.offset(j);
      const Uint var_end = var_begin + var_descriptor.var_length(j);
      for(Uint k = var_begin; k != var_end; ++k)
      {
        const int lid = m_p2m[i*m_neq+k];
        if(lid < m_num_my_elements)
        {
          var_gid_sets[j].insert(matrix_global_ids[lid]);
        }
      }
    }
  }

  var_gids.assign(nb_vars, std::vector<int>());
  for(Uint i = 0; i != nb_vars; ++i)
  {
    var_gids[i].assign(var_gid_sets[i].begin(), var_gid_sets[i].end());
  }
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

Teuchos::RCP< const Thyra::LinearOpBase< Real > > TrilinosCrsMatrix::thyra_operator() const
{
  return Thyra::epetraLinearOp(m_mat);
}

////////////////////////////////////////////////////////////////////////////////////////////

Teuchos::RCP< Thyra::LinearOpBase< Real > > TrilinosCrsMatrix::thyra_operator()
{
  return Thyra::nonconstEpetraLinearOp(m_mat);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosCrsMatrix::apply ( const Handle< Vector >& y, const cf3::Handle< const Vector >& x, const Real alpha, const Real beta )
{
  apply_matrix(*m_mat, y, x, alpha, beta);
}


////////////////////////////////////////////////////////////////////////////////////////////

