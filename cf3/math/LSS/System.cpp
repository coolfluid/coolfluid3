// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <fstream>

#include <boost/utility.hpp>

#include "math/LSS/LibLSS.hpp"

#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/Component.hpp"
#include "common/OptionT.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"
#include <common/PropertyList.hpp>

#include "math/LSS/System.hpp"
#include "math/LSS/Matrix.hpp"
#include "math/LSS/Vector.hpp"
#include "math/LSS/BlockAccumulator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file System.cpp implementation of LSS::System
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////////////////

#include "math/LSS/EmptyLSS/EmptyLSSVector.hpp"
#include "math/LSS/EmptyLSS/EmptyLSSMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LSS::System, LSS::System, LSS::LibLSS > System_Builder;

LSS::System::System(const std::string& name) :
  Component(name)
{
  options().add( "matrix_builder" , "cf3.math.LSS.TrilinosFEVbrMatrix")
    .pretty_name("Matrix Builder")
    .description("Name for the builder used to create the LSS matrix")
    .mark_basic();

  options().add( "vector_builder" , "")
    .pretty_name("Vector Builder")
    .description("Name for the builder used for the vectors. If left empty, this is obtained from the vector_type property of the matrix")
    .mark_basic();

  options().add("solution_strategy", "cf3.math.LSS.TrilinosStratimikosStrategy")
    .pretty_name("Solution Strategy")
    .description("Name of the builder that will be used to create the solution strategy")
    .mark_basic();

  regist_signal("print_system")
    .connect(boost::bind( &System::signal_print, this, _1 ))
    .description("Write the system to disk as a tecplot file, for debugging purposes.")
    .pretty_name("Print System")
    .signature(boost::bind( &System::signature_print, this, _1 ));
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::create(cf3::common::PE::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
{
  if (is_created())
    destroy();

  const std::string matrix_builder = options().option("matrix_builder").value_str();
  m_mat = create_component<LSS::Matrix>("Matrix", matrix_builder);

  std::string vector_builder = options().option("vector_builder").value_str();
  if(vector_builder.empty())
    vector_builder = m_mat->properties().value_str("vector_type");

  m_rhs = create_component<LSS::Vector>("RHS", vector_builder);
  m_sol = create_component<LSS::Vector>("Solution", vector_builder);

  m_rhs->create(cp,neq);
  m_sol->create(cp,neq);
  m_mat->create(cp,neq,node_connectivity,starting_indices,*m_sol,*m_rhs);

  m_rhs->mark_basic();
  m_sol->mark_basic();
  m_mat->mark_basic();

  m_solution_strategy = create_component<SolutionStrategy>("SolutionStrategy", options().option("solution_strategy").value<std::string>());
  m_solution_strategy->set_matrix(m_mat);
  m_solution_strategy->set_solution(m_sol);
  m_solution_strategy->set_rhs(m_rhs);
  m_solution_strategy->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, std::vector< Uint >& node_connectivity, std::vector< Uint >& starting_indices)
{
  if (is_created())
    destroy();

  const std::string matrix_builder = options().option("matrix_builder").value_str();
  m_mat = create_component<LSS::Matrix>("Matrix", matrix_builder);

  std::string vector_builder = options().option("vector_builder").value_str();
  if(vector_builder.empty())
    vector_builder = m_mat->properties().value_str("vector_type");

  m_rhs = create_component<LSS::Vector>("RHS", vector_builder);
  m_sol = create_component<LSS::Vector>("Solution", vector_builder);

  m_rhs->create_blocked(cp,vars);
  m_sol->create_blocked(cp,vars);
  m_mat->create_blocked(cp,vars,node_connectivity,starting_indices,*m_sol,*m_rhs);

  m_rhs->mark_basic();
  m_sol->mark_basic();
  m_mat->mark_basic();

  m_solution_strategy = create_component<SolutionStrategy>("SolutionStrategy", options().option("solution_strategy").value<std::string>());
  m_solution_strategy->set_matrix(m_mat);
  m_solution_strategy->set_solution(m_sol);
  m_solution_strategy->set_rhs(m_rhs);
  m_solution_strategy->mark_basic();
}


////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::swap( const boost::shared_ptr< LSS::Matrix >& matrix, const boost::shared_ptr< LSS::Vector >& solution, const boost::shared_ptr< LSS::Vector >& rhs )
{
  if (m_mat->is_swappable(*solution,*rhs))
  {
  if ((matrix->is_created()!=solution->is_created())||(matrix->is_created()!=rhs->is_created()))
    throw common::SetupError(FromHere(),"Inconsistent states.");
  if ((matrix->solvertype()!=solution->solvertype())||(matrix->solvertype()!=rhs->solvertype()))
    throw common::NotSupported(FromHere(),"Inconsistent linear solver types.");
  if ((matrix->neq()!=solution->neq())||(matrix->neq()!=rhs->neq()))
    throw common::BadValue(FromHere(),"Inconsistent number of equations.");
  if ((matrix->blockcol_size()!=solution->blockrow_size())||(matrix->blockcol_size()!=rhs->blockrow_size()))
    throw common::BadValue(FromHere(),"Inconsistent number of block rows.");

  if(is_not_null(get_child("Matrix")))
    remove_component("Matrix");
  if(is_not_null(get_child("Solution")))
    remove_component("Solution");
  if(is_not_null(get_child("RHS")))
    remove_component("RHS");

  m_mat = make_handle(matrix);
  m_rhs = make_handle(rhs);
  m_sol = make_handle(solution);

  add_component(matrix);
  add_component(solution);
  add_component(rhs);

  options().option("matrix_builder").change_value(matrix->solvertype());
  } else {
    throw common::NotSupported(FromHere(),"System of '" + matrix->name() + "' x '" + solution->name() + "' = '" + rhs->name() + "' is incompatible." );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::destroy()
{
  if(is_not_null(get_child("SolutionStrategy")))
    remove_component("SolutionStrategy");
  if(is_not_null(get_child("Matrix")))
    remove_component("Matrix");
  if(is_not_null(get_child("Solution")))
    remove_component("Solution");
  if(is_not_null(get_child("RHS")))
    remove_component("RHS");

  m_solution_strategy.reset();
  m_mat.reset();
  m_sol.reset();
  m_rhs.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::solve()
{
  cf3_assert(is_created());
  m_solution_strategy->solve();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::set_values(const LSS::BlockAccumulator& values)
{
  cf3_assert(is_created());
  m_mat->set_values(values);
  m_sol->set_sol_values(values);
  m_rhs->set_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::add_values(const LSS::BlockAccumulator& values)
{
  cf3_assert(is_created());
  m_mat->add_values(values);
  m_sol->add_sol_values(values);
  m_rhs->add_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::get_values(LSS::BlockAccumulator& values)
{
  cf3_assert(is_created());
  m_mat->get_values(values);
  m_sol->get_sol_values(values);
  m_rhs->get_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::dirichlet(const Uint iblockrow, const Uint ieq, const Real value, const bool preserve_symmetry)
{
  cf3_assert(is_created());

  if (preserve_symmetry)
  {
    m_mat->symmetric_dirichlet(iblockrow, ieq, value, *m_rhs);
  }
  else
  {
    m_mat->set_row(iblockrow,ieq,1.,0.);
    m_rhs->set_value(iblockrow,ieq,value);
  }

  m_sol->set_value(iblockrow,ieq,value);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::periodicity (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf3_assert(is_created());
  LSS::BlockAccumulator ba;
  const int neq=m_mat->neq();
  ba.resize(2,neq);
  ba.indices[0]=iblockrow_to;
  ba.indices[1]=iblockrow_from;
  m_mat->tie_blockrow_pairs(iblockrow_to,iblockrow_from);
  m_rhs->get_rhs_values(ba);
  for (int i=0; i<(const int)neq; i++)
  {
    ba.rhs[i]+=ba.rhs[neq+i];
    ba.rhs[neq+i]=0.;
  }
  m_rhs->set_rhs_values(ba);
  m_sol->get_sol_values(ba);
  for (int i=0; i<(const int)neq; i++)
  {
    ba.sol[neq+i]=0.5*(ba.sol[i]+ba.sol[neq+i]);
    ba.sol[i]=ba.sol[neq+i];
  }
  m_sol->set_sol_values(ba);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::set_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(is_created());
  m_mat->set_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::add_diagonal(const std::vector<Real>& diag)
{
  cf3_assert(is_created());
  m_mat->add_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::get_diagonal(std::vector<Real>& diag)
{
  cf3_assert(is_created());
  m_mat->get_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::reset(Real reset_to)
{
  cf3_assert(is_created());
  m_mat->reset(reset_to);
  m_sol->reset(reset_to);
  m_rhs->reset(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(common::LogStream& stream)
{
  if (is_created())
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(std::ostream& stream)
{
  if (is_created())
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::print(const std::string& filename)
{
  if (is_created())
  {
    m_mat->print(filename,std::ios_base::out);
    m_sol->print(filename,std::ios_base::app);
    m_rhs->print(filename,std::ios_base::app);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

const bool LSS::System::is_created()
{
  int numcreated=0;
  if (m_mat!=nullptr) if (m_mat->is_created()) numcreated+=1;
  if (m_sol!=nullptr) if (m_sol->is_created()) numcreated+=2;
  if (m_rhs!=nullptr) if (m_rhs->is_created()) numcreated+=4;
  if (m_solution_strategy!=nullptr) numcreated+=8;
  switch (numcreated) {
    case 0 : return false;
    case 15 : return true;
    default: throw common::SetupError(FromHere(),"LSS System is in inconsistent state: " + common::to_str(numcreated));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::signal_print(common::SignalArgs& args)
{
  common::XML::SignalOptions options( args );
  std::string filename = options.value<std::string>( "file_name" );
  print(filename);
}

////////////////////////////////////////////////////////////////////////////////////////////

void LSS::System::signature_print(common::SignalArgs& args)
{
  common::XML::SignalOptions options( args );

  options.add<std::string>("file_name")
    .pretty_name("File name")
    .description("tecplot file to print the matrix to");
}

////////////////////////////////////////////////////////////////////////////////////////////
