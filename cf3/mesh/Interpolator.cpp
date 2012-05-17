// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "math/MatrixTypesConversion.hpp"

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "common/PE/debug.hpp"

#include "mesh/Interpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

#include "mesh/PointInterpolator.hpp"


namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

Interpolator::Interpolator(const std::string &name) : Component(name)
{
  m_point_interpolator = create_component<PointInterpolator>("point_interpolator");

  options().add_option("store", true)
      .description("Flag to store weights and stencils used for faster interpolation")
      .pretty_name("Store");
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
void Interpolator_send_receive(const Uint send_to_pid, std::vector<T>& send, const Uint receive_from_pid, std::vector<T>& receive)
{
  if (send_to_pid == PE::Comm::instance().rank() &&
      receive_from_pid == PE::Comm::instance().rank())
  {
    receive = send;
      return;
  }
  cf3_assert(send.empty()==false);
  size_t recv_size;
  size_t send_size = send.size();
  MPI_Sendrecv(&send_size, 1, PE::get_mpi_datatype<size_t>(), (int)send_to_pid, 0,
               &recv_size, 1, PE::get_mpi_datatype<size_t>(), (int)receive_from_pid, 0,
               PE::Comm::instance().communicator(), MPI_STATUS_IGNORE);
  receive.resize(recv_size);
  MPI_Sendrecv(&send[0], (int)send.size(), PE::get_mpi_datatype<T>(), (int)send_to_pid, 0,
               &receive[0], (int)receive.size(), PE::get_mpi_datatype<T>(), (int)receive_from_pid, 0,
               PE::Comm::instance().communicator(), MPI_STATUS_IGNORE);
}

////////////////////////////////////////////////////////////////////////////////


void Interpolator::store(const Dictionary& dict, const Table<Real>& target_coords)
{
  m_dict  = dict.handle<Dictionary>();
  m_table = target_coords.handle< Table<Real> >();

  cf3_assert(m_point_interpolator);
  m_point_interpolator->options().configure_option("source", const_cast<Dictionary*>(m_dict.get())->handle<Dictionary>());

  Uint nb_coords = target_coords.size();
  Uint dim = target_coords.row_size();

  std::vector<Uint> not_found; not_found.reserve(nb_coords);
  for (Uint i=0; i<nb_coords; ++i)
    not_found.push_back(i);

  m_proc.resize(nb_coords,-1);

  m_proc.clear();
  m_expect_recv.clear();
  m_stored_element.clear();
  m_stored_stencil.clear();
  m_stored_source_field_points.clear();
  m_stored_source_field_weights.clear();

  m_expect_recv.resize(PE::Comm::instance().size());
  m_stored_element.resize(PE::Comm::instance().size());
  m_stored_stencil.resize(PE::Comm::instance().size());
  m_stored_source_field_points.resize(PE::Comm::instance().size());
  m_stored_source_field_weights.resize(PE::Comm::instance().size());


  // Now find missing on other procs.
  for (Uint pid=0; pid<PE::Comm::instance().size(); pid++)
  {
    // Trade my requests with processor send and recv
    const Uint pid_send_coords = (PE::Comm::instance().rank() + pid) %
                                  PE::Comm::instance().size();
    const Uint pid_recv_coords = (PE::Comm::instance().size() + PE::Comm::instance().rank() - pid) %
                                  PE::Comm::instance().size();

    std::vector<Real> send_coords; send_coords.reserve(not_found.size()*target_coords.row_size());
    std::vector<Real> received_coords;

    // fill in coords to send
    boost_foreach (const Uint t, not_found)
    {
      boost_foreach (const Real& xyz, target_coords[t])
      {
        send_coords.push_back(xyz);
      }
    }

    // send coords, and receive coords
    Interpolator_send_receive(pid_send_coords,   send_coords,
                              pid_recv_coords,   received_coords);

    Uint nb_received_coords = received_coords.size()/dim;

    // Find interpolated

    m_stored_element[pid_recv_coords].clear();
    m_stored_stencil[pid_recv_coords].clear();
    m_stored_source_field_points[pid_recv_coords].clear();
    m_stored_source_field_weights[pid_recv_coords].clear();

    m_stored_element[pid_recv_coords].reserve(nb_received_coords);
    m_stored_stencil[pid_recv_coords].reserve(nb_received_coords);
    m_stored_source_field_points[pid_recv_coords].reserve(nb_received_coords);
    m_stored_source_field_weights[pid_recv_coords].reserve(nb_received_coords);

    std::vector<Uint> send_found_coords;  send_found_coords.reserve(nb_received_coords);

    Uint dim = target_coords.row_size();
    RealVector t_point(dim);
    SpaceElem element;
    std::vector<SpaceElem> stencil;
    std::vector<Uint> points;
    std::vector<Real> weights;

    for (Uint t=0; t<nb_received_coords; ++t)
    {
      t_point = RealVector::MapType(&received_coords[t*dim],dim);
      bool interpolation_possible_on_this_proc =
          m_point_interpolator->compute_storage(t_point,
                                                element,
                                                stencil,
                                                points,
                                                weights);

      if (interpolation_possible_on_this_proc)
      {
        m_stored_element[pid_recv_coords].push_back(element);
        m_stored_stencil[pid_recv_coords].push_back(stencil);
        m_stored_source_field_points[pid_recv_coords].push_back(points);
        m_stored_source_field_weights[pid_recv_coords].push_back(weights);

        // mark found
        send_found_coords.push_back(t);
      }

    }

    std::vector<Uint> recv_found_coords;

    const Uint pid_send_back = pid_recv_coords;
    const Uint pid_recv_back = pid_send_coords;

    Interpolator_send_receive (pid_send_back, send_found_coords,
                               pid_recv_back, recv_found_coords);

    m_expect_recv[pid_recv_back].reserve(recv_found_coords.size());

    boost_foreach(const Uint i, recv_found_coords)
    {
      cf3_assert(i<not_found.size());
      const Uint t = not_found[i];
      cf3_assert(t<nb_coords);
      m_proc[t] = pid_recv_back;
      m_expect_recv[pid_recv_back].push_back(t);
    }

    not_found.clear();
    for (Uint t=0; t<nb_coords; ++t)
    {
      if (m_proc[t]<0)
        not_found.push_back(t);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::stored_interpolation(const Field& source_field, Table<Real>& target)
{
  // Do interpolation on processors that can do the interpolation,
  // and send back an array of interpolated values
  for (Uint pid=0; pid<PE::Comm::instance().size(); pid++)
  {
    // Trade my requests with processor send and recv
    const Uint pid_send_interpolated = (PE::Comm::instance().size() + PE::Comm::instance().rank() - pid) %
                                        PE::Comm::instance().size();
    const Uint pid_recv_interpolated = (PE::Comm::instance().rank() + pid) %
                                        PE::Comm::instance().size();

    // number of points to be interpolated
    const Uint nb_points = m_stored_element[pid_send_interpolated].size();
    // number of variables for each point to be interpolated
    const Uint nb_vars = source_field.row_size();

    // storage for interpolated variables, which will be sent to the pid that reqests it (pid_recv_interpolated)
    std::vector<Real> interpolated; interpolated.reserve(nb_points*nb_vars);

    // Interpolation points and weights
    const std::vector< std::vector<Uint> >& s_points  = m_stored_source_field_points[pid_send_interpolated];
    const std::vector< std::vector<Real> >& s_weights = m_stored_source_field_weights[pid_send_interpolated];

    // Do interpolation
    for (Uint t=0; t<nb_points; ++t)
    {
      for (Uint v=0; v<nb_vars; ++v)
      {
        interpolated.push_back(0.);
        for (Uint s=0; s<s_points[t].size(); ++s)
        {
          cf3_assert(s_points[t][s]<source_field.size());
          interpolated.back() += source_field[ s_points[t][s] ][v] * s_weights[t][s];
        }
      }
    }

    // Send/Receive interpolated variables
    std::vector<Real> recv_interpolated;
    Interpolator_send_receive(pid_send_interpolated   , interpolated,
                              pid_recv_interpolated   , recv_interpolated);

    // Fill the target_field with received interpolated variables from requested processor
    Uint it=0;
    boost_foreach( const Uint t, m_expect_recv[pid_recv_interpolated] )
    {
      for (Uint v=0; v<nb_vars; ++v)
      {
        cf3_assert(t<target.size());
        target[t][v] = recv_interpolated[it++];
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::unstored_interpolation(const Field& source_field, const common::Table<Real>& target_coords, common::Table<Real>& target)
{
  // This ensures that storage will need to be recomputed in the future
  m_dict.reset();
  m_table.reset();

  cf3_assert(m_point_interpolator);
  m_point_interpolator->options().configure_option("source", const_cast<Dictionary*>(&source_field.dict())->handle<Dictionary>());

  const Uint nb_coords = target_coords.size();
  const Uint dim = target_coords.row_size();

  std::vector<int> not_found; not_found.reserve(nb_coords);
  for (Uint i=0; i<nb_coords; ++i)
    not_found.push_back(i);

  // Now find missing on other procs.
  for (Uint pid=0; pid<PE::Comm::instance().size(); pid++)
  {
    // Trade my requests with processor send and recv
    const Uint pid_send_coords = (PE::Comm::instance().rank() + pid) %
                                  PE::Comm::instance().size();
    const Uint pid_recv_coords = (PE::Comm::instance().size() + PE::Comm::instance().rank() - pid) %
                                  PE::Comm::instance().size();

    std::vector<Real> send_coords; send_coords.reserve(not_found.size()*target_coords.row_size());
    std::vector<Real> received_coords;

    // fill in coords to send
    boost_foreach (const int t, not_found)
    {
      if (t>=0)
      {
        boost_foreach (const Real& xyz, target_coords[t])
        {
          send_coords.push_back(xyz);
        }
      }
    }

    // send coords, and receive coords
    Interpolator_send_receive(pid_send_coords,   send_coords,
                              pid_recv_coords,   received_coords);

    Uint nb_received_coords = received_coords.size()/dim;

    // Find interpolated

    std::vector<Uint> send_found_coords;  send_found_coords.reserve(nb_received_coords);

    // number of variables for each point to be interpolated
    const Uint nb_vars = source_field.row_size();

    // storage for interpolated variables, which will be sent to the pid that reqests it (pid_recv_interpolated)
    std::vector<Real> send_interpolated; send_interpolated.reserve(nb_received_coords*nb_vars);

    Uint dim = target_coords.row_size();
    RealVector t_point(dim);
    RealVector t_val(nb_vars);

    for (Uint t=0; t<nb_received_coords; ++t)
    {
      t_point = RealVector::MapType(&received_coords[t*dim],dim);
      bool interpolation_possible_on_this_proc =
          m_point_interpolator->interpolate(source_field,t_point,t_val);
      if (interpolation_possible_on_this_proc)
      {
        // mark found
        send_found_coords.push_back(t);

        for (Uint v=0; v<nb_vars; ++v)
          send_interpolated.push_back(t_val[v]);
      }
    }

    std::vector<Uint> recv_found_coords;
    std::vector<Real> recv_interpolated;

    const Uint pid_send_back = pid_recv_coords;
    const Uint pid_recv_back = pid_send_coords;

    Interpolator_send_receive (pid_send_back, send_found_coords,
                               pid_recv_back, recv_found_coords);
    Interpolator_send_receive (pid_send_back, send_interpolated,
                               pid_recv_back, recv_interpolated);

    Uint it=0;
    boost_foreach(const Uint i, recv_found_coords)
    {
      cf3_assert(i<not_found.size());
      const int t = not_found[i];
      not_found[i] = -1;
      cf3_assert(t<nb_coords);
      for (Uint v=0; v<nb_vars; ++v)
      {
        cf3_assert(t<target.size());
        target[t][v] = recv_interpolated[it++];
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

void Interpolator::interpolate(const Field& source_field, Field& target_field)
{
  if (options().option("store").value<bool>())
  {
    if ( is_null(m_dict) || is_null(m_table) )
    {
      store(source_field.dict(),target_field.coordinates());
    }
    if ( m_dict != source_field.handle<Dictionary>() && m_table != target_field.coordinates().handle< Table<Real> >())
    {
      store(source_field.dict(),target_field.coordinates());
    }
    stored_interpolation(source_field,target_field);
  }
  else
  {
    unstored_interpolation(source_field,target_field.coordinates(),target_field);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


OldInterpolator::OldInterpolator ( const std::string& name  ) :
  Component ( name )
{
  options().add_option("source", m_source)
      .description("Field to interpolate from")
      .pretty_name("Source Field")
      .mark_basic()
      .link_to(&m_source);

  options().add_option("target", m_target)
      .description("Field to interpolate to")
      .pretty_name("TargetField")
      .mark_basic()
      .link_to(&m_target);

  options().add_option("store", true)
      .description("Flag to store weights and stencils used for faster interpolation")
      .pretty_name("Store");
}

////////////////////////////////////////////////////////////////////////////////

OldInterpolator::~OldInterpolator()
{
}

//////////////////////////////////////////////////////////////////////////////

void OldInterpolator::signal_interpolate( SignalArgs& node  )
{
  interpolate();
}

////////////////////////////////////////////////////////////////////////////////

void OldInterpolator::interpolate()
{
  if ( is_null(m_source) )
    throw SetupError (FromHere(), "SourceField option was not set");
  if ( is_null(m_target) )
    throw SetupError (FromHere(), "TargetField option was not set");
  construct_internal_storage(*Handle<Mesh>(m_source->parent()));
  interpolate_field_from_to(*m_source,*m_target);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
