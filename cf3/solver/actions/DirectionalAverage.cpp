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
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"

#include "solver/actions/DirectionalAverage.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < DirectionalAverage, common::Action, LibActions > DirectionalAverage_Builder;

///////////////////////////////////////////////////////////////////////////////////////

namespace detail
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

DirectionalAverage::DirectionalAverage ( const std::string& name ) :
  common::Action(name)
{
  options().add("direction", 0u)
    .pretty_name("Direction")
    .description("Normal direction to the planes along which we average")
    .attach_trigger(boost::bind(&DirectionalAverage::trigger, this))
    .mark_basic();

  options().add("field", Handle<mesh::Field>())
    .pretty_name("Field")
    .description("Field to average")
    .attach_trigger(boost::bind(&DirectionalAverage::trigger, this))
    .mark_basic();

  options().add("threshold", 1e-10)
    .pretty_name("Threshold")
    .description("Threshold to use when comparing coordinates")
    .attach_trigger(boost::bind(&DirectionalAverage::trigger, this))
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name to write the averaged data to")
    .mark_basic();
}

void DirectionalAverage::execute()
{
  setup();

  std::fill(m_averages.begin(), m_averages.end(), 0.);
  
  const mesh::Field::ArrayT& field_values = m_field->array();
  const Uint nb_nodes = field_values.size();
  const mesh::Dictionary& dict = m_field->dict();
  const common::List<bool>* periodic_links_active = Handle<common::List<bool> const>(dict.get_child("periodic_links_active")).get();
  const Uint stride = m_field->row_size()+1;
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    if(!dict.is_ghost(node_idx) && !(is_not_null(periodic_links_active) && (*periodic_links_active)[node_idx]))
    {
      const Uint position_idx = m_node_position_indices[node_idx];
      const Uint node_begin = stride*position_idx;
      ++m_averages[node_begin];
      const mesh::Field::ConstRow row = field_values[node_idx];
      std::transform(row.begin(), row.end(), m_averages.begin()+node_begin+1, m_averages.begin()+node_begin+1, std::plus<Real>());
    }
  }
  
  common::PE::Comm& comm = common::PE::Comm::instance();
  std::vector<Real> global_averages;
  if(comm.is_active())
  {
    comm.reduce(common::PE::plus(), m_averages, global_averages, 0);
  }
  else
  {
    global_averages = m_averages;
  }
  
  if(comm.rank() == 0)
  {
    boost::filesystem::fstream file(options().value<common::URI>("file").path(), std::ios::out);
    const Uint nb_positions = m_positions.size();
    file << "# Position, Count, Averages for " << m_field->descriptor().description() << "\n";
    for(Uint pos_idx = 0; pos_idx != nb_positions; ++pos_idx)
    {
      const Uint avg_begin = pos_idx*stride;
      const Uint avg_end = avg_begin+stride;
      const Real count = global_averages[avg_begin];
      file << common::to_str(m_positions[pos_idx]) << " " << count;

      for(Uint j = avg_begin+1; j != avg_end; ++j)
      {
        file << " " << common::to_str(global_averages[j]/count);
      }
      file << "\n";
    }
    file.close();
  }
}

void DirectionalAverage::trigger()
{
  m_field.reset();
}

void DirectionalAverage::setup()
{
  if(is_not_null(m_field))
    return;

  m_field = options().value< Handle<mesh::Field> >("field");
  if(is_null(m_field))
    throw common::SetupError(FromHere(), "No field configured for " + uri().path());

  const mesh::Dictionary& dict = m_field->dict();
  const Uint nb_nodes = dict.size();
  const mesh::Field& coords = dict.coordinates();

  const Uint direction = options().value<Uint>("direction");
  if(direction >= coords.row_size())
    throw common::SetupError(FromHere(), "Direction " + common::to_str(direction) + " is not allowed for mesh of dimension " + common::to_str(coords.row_size()));

  std::set<Real, detail::threshold_compare> unique_coords(detail::threshold_compare(options().value<Real>("threshold")));

  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx )
  {
    if(!dict.is_ghost( node_idx ))
      unique_coords.insert(coords[node_idx][direction]);
  }

  common::PE::Comm& comm = common::PE::Comm::instance();
  if(comm.is_active())
  {
    std::vector<Real> my_unique_coords(unique_coords.begin(), unique_coords.end());
    std::vector< std::vector<Real> > gathered_coords;
    comm.all_gather(my_unique_coords, gathered_coords);
    BOOST_FOREACH(const std::vector<Real>& vec, gathered_coords)
    {
      unique_coords.insert(vec.begin(), vec.end());
    }
  }
  
  std::map<Real, Uint, detail::threshold_compare> coords_map(detail::threshold_compare(options().value<Real>("threshold")));
  Uint coord_gid = 0;
  BOOST_FOREACH(const Real coord, unique_coords)
  {
    coords_map[coord] = coord_gid++;
  }
  
  m_positions.assign(unique_coords.begin(), unique_coords.end());

  CFinfo << "Found " << m_positions.size() << " unique coordinates in direction " << direction << CFendl;
  
  m_node_position_indices.resize(nb_nodes);
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx )
  {
    if(!dict.is_ghost( node_idx ))
    {
      cf3_assert(coords_map.find(coords[node_idx][direction]) != coords_map.end());
      m_node_position_indices[node_idx] = coords_map[coords[node_idx][direction]];
    }
  }
  
  // For each position, we store the count and then the sum of all values in the field
  m_averages.resize(m_positions.size() * (m_field->row_size()+1));
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

