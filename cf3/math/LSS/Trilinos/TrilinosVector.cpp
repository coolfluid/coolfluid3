// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/Assertions.hpp"
#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/LSS/Trilinos/TrilinosDetail.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "Thyra_EpetraThyraWrappers.hpp"

/// @todo remove when no debug any more
#include "common/PE/debug.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosVector.cpp Implementation of LSS::vector interface for Trilinos package.
  @author Tamas Banyai

  The chosen tool is epetra vector which has been implemented.
**/

////////////////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::LSS;

common::ComponentBuilder < LSS::TrilinosVector, LSS::Vector, LSS::LibLSS > TrilinosVector_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

TrilinosVector::TrilinosVector(const std::string& name) :
  LSS::Vector(name),
  m_neq(0),
  m_blockrow_size(0),
  m_is_created(false),
  m_vec(0),
  m_converted_indices(0),
  m_comm(common::PE::Comm::instance().communicator())
{
  regist_signal( "print_native" )
      .connect( boost::bind( &TrilinosVector::signal_print_native, this, _1 ) )
      .description("Prints the native representation of the vector")
      .pretty_name("Print Native");
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::create(common::PE::CommPattern& cp, Uint neq, const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active)
{
  boost::shared_ptr<VariablesDescriptor> single_var_descriptor = common::allocate_component<VariablesDescriptor>("SingleVariableDescriptor");
  single_var_descriptor->options().set(common::Tags::dimension(), neq);
  single_var_descriptor->push_back("LSSvars", VariablesDescriptor::Dimensionalities::VECTOR);
  create_blocked(cp, *single_var_descriptor, periodic_links_nodes, periodic_links_active);
}

void TrilinosVector::create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active)
{
  // if built
  if (m_is_created) destroy();

  // prepare intermediate data
  int nmyglobalelements=0;
  std::vector<int> myglobalelements(0);

  std::vector<Uint> my_ranks;
  create_map_data(cp, vars, m_p2m, myglobalelements, my_ranks, nmyglobalelements, periodic_links_nodes, periodic_links_active);

  m_data.resize(myglobalelements.size());

  std::vector<Uint> gids(myglobalelements.begin(), myglobalelements.end()); // need Uint data for GIDs

  if(is_not_null(get_child("CommPattern")))
    remove_component("CommPattern");
  m_comm_pattern = common::allocate_component<common::PE::CommPattern>("CommPattern");
  m_comm_pattern->insert("gid",gids,1,false);
  m_comm_pattern->setup(Handle<common::PE::CommWrapper>(m_comm_pattern->get_child("gid")),my_ranks);

  m_comm_pattern->insert(name(), m_data, true);
  
  // map (its actually blockmap insteady of rowmap, to involve ghosts)
  m_map = Teuchos::rcp(new Epetra_Map(-1,nmyglobalelements,&myglobalelements[0],0,m_comm));
  // create vector
  m_vec=Teuchos::rcp(new Epetra_Vector(View, *m_map, &m_data[0]));

  m_neq=vars.size();
  m_blockrow_size=cp.isUpdatable().size();
  m_is_created=true;
}


////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::destroy()
{
  if (m_is_created) m_vec.reset();
  m_p2m.resize(0);
  m_p2m.reserve(0);
  m_neq=0;
  m_blockrow_size=0;
  m_is_created=false;
  m_data.resize(0);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_value(const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  m_data[m_p2m[irow]]=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_value(const Uint irow, const Real value)
{
  cf3_assert(m_is_created);
  m_data[m_p2m[irow]]+=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_value(const Uint irow, Real& value)
{
  cf3_assert(m_is_created);
  value=m_data[m_p2m[irow]];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_value(const Uint iblockrow, const Uint ieq, const Real value)
{
  cf3_assert(m_is_created);
  cf3_assert(iblockrow<m_blockrow_size);
  m_data[m_p2m[iblockrow*m_neq+ieq]]=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_value(const Uint iblockrow, const Uint ieq, const Real value)
{
  cf3_assert(m_is_created);
  cf3_assert(iblockrow<m_blockrow_size);
  m_data[m_p2m[iblockrow*m_neq+ieq]]+=value;
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_value(const Uint iblockrow, const Uint ieq, Real& value)
{
  cf3_assert(m_is_created);
  cf3_assert(iblockrow<m_blockrow_size);
  value=m_data[m_p2m[iblockrow*m_neq+ieq]];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_rhs_values(const BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.rhs[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    for (int j=0; j<(const int)m_neq; j++)
      m_data[m_p2m[values.indices[i]*m_neq+j]]=*vals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_rhs_values(const BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.rhs[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    cf3_assert(values.indices[i] >= 0);
    cf3_assert(values.indices[i] < m_blockrow_size);
    for (int j=0; j<(const int)m_neq; j++)
      m_data[m_p2m[values.indices[i]*m_neq+j]]+=*vals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_rhs_values(BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.rhs[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    cf3_assert(values.indices[i] >= 0);
    cf3_assert(values.indices[i] < m_blockrow_size);
    for (int j=0; j<(const int)m_neq; j++)
      *vals++=m_data[m_p2m[values.indices[i]*m_neq+j]];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set_sol_values(const BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.sol[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    cf3_assert(values.indices[i] >= 0);
    cf3_assert(values.indices[i] < m_blockrow_size);
    for (int j=0; j<(const int)m_neq; j++)
      m_data[m_p2m[values.indices[i]*m_neq+j]]=*vals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::add_sol_values(const BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.sol[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    cf3_assert(values.indices[i] >= 0);
    cf3_assert(values.indices[i] < m_blockrow_size);
    for (int j=0; j<(const int)m_neq; j++)
      m_data[m_p2m[values.indices[i]*m_neq+j]]+=*vals++;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get_sol_values(BlockAccumulator& values)
{
  /// @note looked up the code and access mechanism is a mess, much less cpu to access here in a for loop and directly do whats desired
  cf3_assert(m_is_created);
  const int numblocks=values.indices.size();
  if (m_converted_indices.size()<numblocks) m_converted_indices.resize(numblocks);
  int *conv=&m_converted_indices[0];
  double *vals=(double*)&values.sol[0];
  for (int i=0; i<(const int)numblocks; i++)
  {
    cf3_assert(values.indices[i] >= 0);
    cf3_assert(values.indices[i] < m_blockrow_size);
    for (int j=0; j<(const int)m_neq; j++)
      *vals++=m_data[m_p2m[values.indices[i]*m_neq+j]];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::reset(Real reset_to)
{
  cf3_assert(m_is_created);
  m_data.assign(m_data.size(), reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::get( boost::multi_array<Real, 2>& data)
{
  cf3_assert(m_is_created);
  cf3_assert(data.shape()[0]==m_blockrow_size);
  cf3_assert(data.shape()[1]==m_neq);
  for (int i=0; i<(const int)m_blockrow_size; i++)
    for (int j=0; j<(const int)m_neq; j++)
      data[i][j]=m_data[m_p2m[i*m_neq+j]];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::set( boost::multi_array<Real, 2>& data)
{
  cf3_assert(m_is_created);
  cf3_assert(data.shape()[0]==m_blockrow_size);
  cf3_assert(data.shape()[1]==m_neq);
  for (int i=0; i<(const int)m_blockrow_size; i++)
    for (int j=0; j<(const int)m_neq; j++)
      m_data[m_p2m[i*m_neq+j]]=data[i][j];
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(common::LogStream& stream)
{
  if (m_is_created)
  {
    for (int i=0; i<(const int)m_blockrow_size; i++)
      for (int j=0; j<(const int)m_neq; j++)
        stream << 0 << " " << -(int)(i*m_neq+j) << " " << m_data[m_p2m[i*m_neq+j]] << "\n";
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_blockrow_size*m_neq << "\n";
    stream << "# number of block rows: " << m_blockrow_size << "\n";
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(std::ostream& stream)
{
  if (m_is_created)
  {
    for (int i=0; i<(const int)m_blockrow_size; i++)
      for (int j=0; j<(const int)m_neq; j++)
        stream << 0 << " " << -(int)(i*m_neq+j) << " " << m_data[m_p2m[i*m_neq+j]] << "\n" << std::flush;
    stream << "# name:                 " << name() << "\n";
    stream << "# type_name:            " << type_name() << "\n";
    stream << "# process:              " << m_comm.MyPID() << "\n";
    stream << "# number of equations:  " << m_neq << "\n";
    stream << "# number of rows:       " << m_blockrow_size*m_neq << "\n";
    stream << "# number of block rows: " << m_blockrow_size << "\n" << std::flush;
  } else {
    stream << name() << " of type " << type_name() << "::is_created() is false, nothing is printed.";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print(const std::string& filename, std::ios_base::openmode mode)
{
  std::ofstream stream(filename.c_str(),mode);
  stream << "VARIABLES=COL,ROW,VAL\n" << std::flush;
  stream << "ZONE T=\"" << type_name() << "::" << name() <<  "\"\n" << std::flush;
  print(stream);
  stream.close();
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::print_native(ostream& stream)
{
  m_vec->Print(stream);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::debug_data(std::vector<Real>& values)
{
  cf3_assert(m_is_created);
  values.clear();
  for (int i=0; i<(const int)m_blockrow_size; i++)
    for (int j=0; j<(const int)m_neq; j++)
      values.push_back(m_data[m_p2m[i*m_neq+j]]);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::signal_print_native(common::SignalArgs& args)
{
  print_native(std::cout);
}

////////////////////////////////////////////////////////////////////////////////////////////

Teuchos::RCP< const Thyra::VectorBase< Real > > TrilinosVector::thyra_vector () const
{
  Teuchos::RCP< const Thyra::VectorSpaceBase< Real > > space = Thyra::create_VectorSpace(m_map);
  return Thyra::create_Vector(m_vec, space);
}


////////////////////////////////////////////////////////////////////////////////////////////

Teuchos::RCP< Thyra::VectorBase< Real > > TrilinosVector::thyra_vector ()
{
  Teuchos::RCP< const Thyra::VectorSpaceBase< Real > > space = Thyra::create_VectorSpace(m_map);
  return Thyra::create_Vector(m_vec, space);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::clone_to(Vector &other)
{
  if(!m_is_created)
    throw common::SetupError(FromHere(), "Vector to clone " + uri().string() + " is not created");

  TrilinosVector* other_ptr = dynamic_cast<TrilinosVector*>(&other);
  if(is_null(other_ptr))
    throw common::SetupError(FromHere(), "clone_to method of TrilinosVector needs another TrilinosVector, but a " + other.derived_type_name() + " was supplied instead.");

  other_ptr->m_data = m_data;
  other_ptr->m_vec = Teuchos::rcp(new Epetra_Vector(View, *m_map, &other_ptr->m_data[0]));
  other_ptr->m_map = m_map;
  other_ptr->m_neq = m_neq;
  other_ptr->m_blockrow_size = m_blockrow_size;
  other_ptr->m_is_created = m_is_created;
  other_ptr->m_p2m = m_p2m;
  other_ptr->m_converted_indices = m_converted_indices;
  other_ptr->m_comm_pattern = m_comm_pattern;
  m_comm_pattern->insert(other_ptr->name(), other_ptr->m_data, true);
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::assign(const Vector& source)
{
  TrilinosVector const* source_ptr = dynamic_cast<TrilinosVector const*>(&source);
  
  if(is_null(source_ptr))
    throw common::SetupError(FromHere(), "assign method of TrilinosVector needs another TrilinosVector, but a " + source.derived_type_name() + " was supplied instead.");
  
  if(source_ptr->m_data.size() != m_data.size())
    throw common::SetupError(FromHere(), "assign method of TrilinosVector got a vector with incorrect size");

  m_data.assign(source_ptr->m_data.begin(), source_ptr->m_data.end());
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::update ( const Vector& source, const Real alpha )
{
  TrilinosVector const* source_ptr = dynamic_cast<TrilinosVector const*>(&source);
  
  if(is_null(source_ptr))
    throw common::SetupError(FromHere(), "assign method of TrilinosVector needs another TrilinosVector, but a " + source.derived_type_name() + " was supplied instead.");
  
  if(source_ptr->m_data.size() != m_data.size())
    throw common::SetupError(FromHere(), "update method of TrilinosVector got a vector with incorrect size");
  
  const Uint size = m_data.size();
  
  if(alpha == 1.)
  {
    for(Uint i = 0; i != size; ++i)
    {
      m_data[i] += source_ptr->m_data[i];
    }
  }
  else
  {
    for(Uint i = 0; i != size; ++i)
    {
      m_data[i] += alpha*source_ptr->m_data[i];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::scale ( const Real alpha )
{
  const Uint size = m_data.size();
  if(alpha != 1.)
  {
    for(Uint i = 0; i != size; ++i)
      m_data[i] *= alpha;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////

void TrilinosVector::sync()
{
  m_comm_pattern->synchronize(name());
}

////////////////////////////////////////////////////////////////////////////////////////////
