// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/PropertyList.hpp"
#include "common/PE/Comm.hpp"
#include <common/PE/CommPattern.hpp>
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"

#include "solver/actions/TwoPointCorrelation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TwoPointCorrelation, common::Action, LibActions > TwoPointCorrelation_Builder;

///////////////////////////////////////////////////////////////////////////////////////

namespace detail_twopoint
{

struct threshold_compare
{
  threshold_compare(const Real threshold) : m_threshold(threshold)
  {
  }

  bool operator()(const Real a, const Real b) const
  {
    return (b - a) > m_threshold;
  }

  const Real m_threshold;
};

}

TwoPointCorrelation::TwoPointCorrelation ( const std::string& name ) :
  common::Action(name),
  m_count(0),
  m_interval(1)
{
  options().add("normal", 1u)
    .pretty_name("Normal")
    .description("Normal direction to the plane in which the correlations are computed")
    .attach_trigger(boost::bind(&TwoPointCorrelation::trigger, this))
    .mark_basic();

  options().add("field", Handle<mesh::Field>())
    .pretty_name("Field")
    .description("Field to consider. The two-point autocorrelation for each variable in the field will be computed")
    .attach_trigger(boost::bind(&TwoPointCorrelation::trigger, this))
    .mark_basic();

  options().add("threshold", 1e-10)
    .pretty_name("Threshold")
    .description("Threshold to use when comparing coordinates")
    .attach_trigger(boost::bind(&TwoPointCorrelation::trigger, this))
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name to write the averaged data to")
    .mark_basic();

  options().add("coordinate", 0.)
    .pretty_name("Coordinate")
    .description("Coordinate in the normal direction")
    .attach_trigger(boost::bind(&TwoPointCorrelation::trigger, this))
    .mark_basic();
    
  options().add("interval", m_interval)
    .pretty_name("Interval")
    .description("Write every interval timesteps")
    .mark_basic()
    .link_to(&m_interval);
}

void TwoPointCorrelation::execute()
{
  setup();
  
  const Uint nb_used_nodes = m_used_node_lids.size();
  
  common::PE::Comm& comm = common::PE::Comm::instance();
  if(comm.is_active() && comm.rank() == m_root)
  {
    const Uint nb_x_gids = m_x_positions.size();
    const Uint nb_y_gids = m_y_positions.size();
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      const Uint gid = m_used_node_y_gids[i]*nb_x_gids+m_used_node_x_gids[i];
      m_sampled_values[gid] = m_field->array()[m_used_node_lids[i]];
    }
  }
  else
  {
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      m_sampled_values[i] = m_field->array()[m_used_node_lids[i]];
    }
  }
  
  if(comm.is_active())
    m_comm_pattern->synchronize("samples");
  
  if(!comm.is_active() || comm.rank() == m_root)
  {
    const Uint nb_x_gids = m_x_positions.size();
    const Uint nb_y_gids = m_y_positions.size();
    
    const RealMatrix old_x_sum = m_x_corr*static_cast<Real>(m_count);
    const RealMatrix old_y_sum = m_y_corr*static_cast<Real>(m_count);
    m_x_corr.setZero();
    m_y_corr.setZero();
    
    const Uint dim = m_field->row_size();
    
    for(Uint i = 0; i != nb_x_gids; ++i)
    {
      const RealRowVector y_ref = Eigen::Map<RealRowVector const>(&m_sampled_values[i][0], dim);
      for(Uint j = 0; j != nb_y_gids; ++j)
      {
        const Uint x_ref_gid = nb_x_gids*j;
        Eigen::Map<RealRowVector const> mapped_x_ref(&m_sampled_values[x_ref_gid][0], dim);
        Eigen::Map<RealRowVector const> mapped_val(&m_sampled_values[x_ref_gid + i][0], dim);
        m_x_corr.row(i).array() += mapped_x_ref.array() * mapped_val.array();
        m_y_corr.row(j).array() += y_ref.array() * mapped_val.array();
      }
    }
    
    ++m_count;
    
    m_x_corr = (m_x_corr/static_cast<Real>(nb_y_gids) + old_x_sum) / static_cast<Real>(m_count);
    m_y_corr = (m_y_corr/static_cast<Real>(nb_x_gids) + old_y_sum) / static_cast<Real>(m_count);
    
    if(m_count % m_interval == 0)
    {
      const Uint normal = options().value<Uint>("normal");
      const Uint x_direction = (normal+1) % 3;
      const Uint y_direction = (normal+2) % 3;
      const Real coord = options().value<Real>("coordinate");
      
      const common::URI original_uri = options().value<common::URI>("file");
      std::string rewritten_path = original_uri.path();
      boost::algorithm::replace_all(rewritten_path, "{iteration}", common::to_str(m_count));
      
      boost::filesystem::fstream file(rewritten_path, std::ios::out);
      file << "# Autocorrelation at level " << coord << " in direction " << x_direction << " for field " << m_field->descriptor().description() << "\n";
      for(Uint i = 0; i != nb_x_gids; ++i)
      {
        file << m_x_positions[i];
        for(Uint j = 0; j != dim; ++j)
          file << "," << common::to_str(m_x_corr(i,j));
        file << "\n";
      }
      file << "# Autocorrelation at level " << coord << " in direction " << y_direction << " for field " << m_field->descriptor().description() << "\n";
      for(Uint i = 0; i != nb_y_gids; ++i)
      {
        file << m_y_positions[i];
        for(Uint j = 0; j != dim; ++j)
          file << "," << common::to_str(m_y_corr(i,j));
        file << "\n";
      }
      file.close();
    }
  }
}

void TwoPointCorrelation::trigger()
{
  m_field.reset();
  m_count = 0;
}

void TwoPointCorrelation::setup()
{
  if(is_not_null(m_field))
    return;

  if(is_not_null(m_comm_pattern))
    remove_component(*m_comm_pattern);

  m_field = options().value< Handle<mesh::Field> >("field");
  if(is_null(m_field))
    throw common::SetupError(FromHere(), "No field configured for " + uri().path());

  const mesh::Dictionary& dict = m_field->dict();
  const Uint nb_nodes = dict.size();
  const mesh::Field& coords = dict.coordinates();

  if(coords.row_size() != 3)
  {
    throw common::SetupError(FromHere(), "TwoPointCorreleation must be used on a 3D problem");
  }

  const Uint normal = options().value<Uint>("normal");
  if(normal >= coords.row_size())
    throw common::SetupError(FromHere(), "normal " + common::to_str(normal) + " is not allowed for mesh of dimension " + common::to_str(coords.row_size()));

  const Uint x_direction = (normal+1) % 3;
  const Uint y_direction = (normal+2) % 3;

  const Real coordinate = options().value<Real>("coordinate");

  const Real threshold = options().value<Real>("threshold");

  std::set<Real, detail_twopoint::threshold_compare> unique_x_coords((detail_twopoint::threshold_compare(threshold)));
  std::set<Real, detail_twopoint::threshold_compare> unique_y_coords((detail_twopoint::threshold_compare(threshold)));
  Uint node_counter = 0;
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx )
  {
    if(!dict.is_ghost( node_idx ) && ::fabs(coords[node_idx][normal] - coordinate) < threshold)
    {
      unique_x_coords.insert(coords[node_idx][x_direction]);
      unique_y_coords.insert(coords[node_idx][y_direction]);
      ++node_counter;
    }
  }
  const Uint nb_used_nodes = node_counter;

  common::PE::Comm& comm = common::PE::Comm::instance();
  
  if(comm.is_active())
  {
    std::vector<Real> my_unique_x_coords(unique_x_coords.begin(), unique_x_coords.end());
    std::vector<Real> my_unique_y_coords(unique_y_coords.begin(), unique_y_coords.end());
    std::vector< std::vector<Real> > gathered_x_coords, gathered_y_coords;
    comm.all_gather(my_unique_x_coords, gathered_x_coords);
    comm.all_gather(my_unique_y_coords, gathered_y_coords);
    BOOST_FOREACH(const std::vector<Real>& vec, gathered_x_coords)
    {
      unique_x_coords.insert(vec.begin(), vec.end());
    }
    BOOST_FOREACH(const std::vector<Real>& vec, gathered_y_coords)
    {
      unique_y_coords.insert(vec.begin(), vec.end());
    }
  }
  
  std::map<Real, Uint, detail_twopoint::threshold_compare> x_coords_map((detail_twopoint::threshold_compare(threshold)));
  std::map<Real, Uint, detail_twopoint::threshold_compare> y_coords_map((detail_twopoint::threshold_compare(threshold)));
  Uint coord_x_gid = 0;
  Uint coord_y_gid = 0;
  BOOST_FOREACH(const Real coord, unique_x_coords)
  {
    x_coords_map[coord] = coord_x_gid++;
  }
  BOOST_FOREACH(const Real coord, unique_y_coords)
  {
    y_coords_map[coord] = coord_y_gid++;
  }
  
  m_x_positions.assign(unique_x_coords.begin(), unique_x_coords.end());
  m_y_positions.assign(unique_y_coords.begin(), unique_y_coords.end());

  CFinfo << "Found " << m_x_positions.size() << "x" << m_y_positions.size() << " unique coordinates in direction normal to " << normal << CFendl;
  
  m_used_node_lids.clear();
  m_used_node_lids.reserve(nb_used_nodes);
  m_used_node_x_gids.resize(nb_used_nodes); m_used_node_y_gids.resize(nb_used_nodes);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx )
  {
    if(!dict.is_ghost( node_idx ) && ::fabs(coords[node_idx][normal] - coordinate) < threshold)
    {
      cf3_assert(x_coords_map.find(coords[node_idx][x_direction]) != x_coords_map.end());
      cf3_assert(y_coords_map.find(coords[node_idx][y_direction]) != y_coords_map.end());
      
      m_used_node_x_gids[m_used_node_lids.size()] = x_coords_map[coords[node_idx][x_direction]];
      m_used_node_y_gids[m_used_node_lids.size()] = y_coords_map[coords[node_idx][y_direction]];
      m_used_node_lids.push_back(node_idx);
    }
  }
  
  const Uint nb_x_gids = m_x_positions.size();
  const Uint nb_y_gids = m_y_positions.size();
  

  
  if(comm.is_active())
  {
    m_root = 0;
    std::vector< std::vector<Uint> > send_gids, recv_gids;
    send_gids.resize(comm.size());
    send_gids[m_root].reserve(nb_used_nodes);
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      send_gids[m_root].push_back(m_used_node_y_gids[i]*nb_x_gids+m_used_node_x_gids[i]);
    }

    comm.all_to_all(send_gids, recv_gids);

    if(comm.rank() == m_root)
    {
      m_ranks.resize(nb_x_gids*nb_y_gids);
      m_gids.resize(nb_x_gids*nb_y_gids);
      for(Uint rank = 0; rank != comm.size(); ++rank)
      {
        BOOST_FOREACH(const Uint gid, recv_gids[rank])
        {
          m_gids[gid] = gid;
          m_ranks[gid] = rank;
        }
      }

      m_sampled_values.resize(boost::extents[nb_x_gids*nb_y_gids][m_field->row_size()]);
      m_x_corr.resize(nb_x_gids, m_field->row_size()); m_x_corr.setZero();
      m_y_corr.resize(nb_y_gids, m_field->row_size()); m_y_corr.setZero();
    }
    else
    {
      m_gids = send_gids[m_root];
      m_ranks.assign(nb_used_nodes, comm.rank());
      m_sampled_values.resize(boost::extents[nb_used_nodes][m_field->row_size()]);
    }
    
    m_comm_pattern = create_component<common::PE::CommPattern>("CommPattern");
    m_comm_pattern->insert("gid", m_gids, 1, false);
    m_comm_pattern->setup(Handle<common::PE::CommWrapper>(m_comm_pattern->get_child("gid")), m_ranks);
    m_comm_pattern->insert("samples", m_sampled_values);
  }
  else
  {
    m_x_corr.resize(nb_x_gids, m_field->row_size()); m_x_corr.setZero();
    m_y_corr.resize(nb_y_gids, m_field->row_size()); m_y_corr.setZero();
    m_sampled_values.resize(boost::extents[nb_x_gids*nb_y_gids][m_field->row_size()]);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

