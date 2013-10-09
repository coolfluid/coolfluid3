// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/PE/Comm.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"

#include "solver/actions/TurbulenceStatistics.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TurbulenceStatistics, common::Action, LibActions > TurbulenceStatistics_Builder;

///////////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  std::vector<Real> gather_vector_field(const std::vector<Uint>& used_nodes, const mesh::Field& field)
  {
    const Uint nb_used_nodes = used_nodes.size();
    std::vector<Real> local_result; local_result.reserve(nb_used_nodes*field.row_size());
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      const mesh::Field::ConstRow row = field[used_nodes[i]];
      local_result.insert(local_result.end(), row.begin(), row.end());
    }
    
    common::PE::Comm& comm = common::PE::Comm::instance();
    if(!comm.is_active())
      return local_result;
    
    std::vector<Real> global_result;
    std::vector<int> recv_counts(comm.size(), -1);
    comm.gather(local_result, local_result.size(), global_result, recv_counts, 0);
    
    return global_result;
  }

  typedef boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::mean> > MeanAccT;
  typedef boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > RollingAccT;

  Real get_mean(const MeanAccT& acc)
  {
    return boost::accumulators::mean(acc);
  }

  Real get_mean(const RollingAccT& acc)
  {
    return boost::accumulators::rolling_mean(acc);
  }

  template<typename AccumulatorsT>
  void append_stats(const Uint i, const Real u, const Real v, AccumulatorsT& acc)
  {
    acc[i  ](u);
    acc[i+1](v);

    const Real up =  u - get_mean(acc[i]);
    const Real vp =  v - get_mean(acc[i+1]);

    acc[i+2](up*up);
    acc[i+3](vp*vp);
    acc[i+4](up*vp);
  }

  template<typename AccumulatorsT>
  void append_stats(const Uint i, const Real u, const Real v, const Real w, AccumulatorsT& acc)
  {
    acc[i  ](u);
    acc[i+1](v);
    acc[i+2](w);

    const Real up =  u - get_mean(acc[i]);
    const Real vp =  v - get_mean(acc[i+1]);
    const Real wp =  w - get_mean(acc[i+2]);

    acc[i+3](up*up);
    acc[i+4](vp*vp);
    acc[i+5](wp*wp);
    acc[i+6](up*vp);
    acc[i+7](up*wp);
    acc[i+8](vp*wp);
  }
}

TurbulenceStatistics::TurbulenceStatistics ( const std::string& name ) :
  common::Action(name),
  m_rolling_size(10),
  m_options_changed(true),
  m_dim(0),
  m_field_offset(0),
  m_count(0)
{
  options().add("variable_name", "Velocity")
    .pretty_name("Variable Name")
    .description("The velocity field for which turbulent statistics are to be calculated")
    .attach_trigger(boost::bind(&TurbulenceStatistics::trigger_option, this))
    .mark_basic();

  options().add("region", Handle<mesh::Region>())
    .pretty_name("Region")
    .description("Region to loop over")
    .attach_trigger(boost::bind(&TurbulenceStatistics::trigger_option, this))
    .mark_basic();

  options().add("rolling_window", 10u)
    .pretty_name("Rolling window")
    .description("Window size for the rolling averages")
    .attach_trigger(boost::bind(&TurbulenceStatistics::reset_statistics, this))
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name for the output file")
    .mark_basic();

  options().add("write_interval", 100u)
    .pretty_name("write_interval")
    .description("Write each N iterations")
    .mark_basic();

  regist_signal( "add_probe" )
    .connect( boost::bind( &TurbulenceStatistics::signal_add_probe, this, _1 ) )
    .description("Add a probe at the given location, logging to its own file")
    .pretty_name("Add Probe")
    .signature( boost::bind( &TurbulenceStatistics::signature_add_probe, this, _1));
}

void TurbulenceStatistics::execute()
{
  setup();
  common::PE::Comm& comm = common::PE::Comm::instance();

  const Uint nb_nodes = m_used_nodes.size();
  const Uint stride = 2.*m_dim + m_dim-1 + m_dim-2;
  const mesh::Field::ArrayT& velocity_array = m_field->array();

  if(m_dim == 2)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const mesh::Field::ConstRow velocity = velocity_array[m_used_nodes[i]];
      const Real u = velocity[XX]; const Real v = velocity[YY];
      detail::append_stats(stride*i, u, v, m_means);
      detail::append_stats(stride*i, u, v, m_rolling_means);
    }
  }
  else if(m_dim == 3)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const mesh::Field::ConstRow velocity = velocity_array[m_used_nodes[i]];
      const Real u = velocity[XX]; const Real v = velocity[YY]; const Real w = velocity[ZZ];
      detail::append_stats(stride*i, u, v, w, m_means);
      detail::append_stats(stride*i, u, v, w, m_rolling_means);
    }
  }

  ++m_count;

  // Write out the statistics if we reach the interval to do so
  const Uint write_interval = options().value<Uint>("write_interval");
  if(m_count % write_interval == 0)
  {
    const int nb_means = m_means.size();
    std::vector<Real> my_means; my_means.reserve(nb_means);
    std::vector<Real> my_rolling_means; my_rolling_means.reserve(nb_means);
    for(Uint i = 0; i != nb_means; ++i)
    {
      my_means.push_back(boost::accumulators::mean(m_means[i]));
      my_rolling_means.push_back(boost::accumulators::rolling_mean(m_rolling_means[i]));
    }
    
    std::vector<Real> global_means;
    std::vector<Real> global_rolling_means;
    std::vector<int> recv_counts(comm.size(), -1);
    if(comm.is_active())
    {
      comm.gather(my_means, nb_means, global_means, recv_counts, 0);
      comm.gather(my_rolling_means, nb_means, global_rolling_means, recv_counts, 0);
    }
    else
    {
      global_means = my_means;
      global_rolling_means = my_rolling_means;
    }
    
    if(comm.rank() == 0)
    {
      std::string out_path = options().value<common::URI>("file").path();
      boost::algorithm::replace_all(out_path, "{iteration}", common::to_str(m_count));
      boost::filesystem::fstream file(out_path, std::ios_base::out);
      if(!file)
        throw common::FileSystemError(FromHere(), "Failed to open file " + out_path);
      
      if(m_dim == 2)
      {
        file << "# U, V, uu, vv, uv, U_rolling, V_rolling, uu_rolling, vv_rolling, uv_rolling\n";
      }
      else if(m_dim == 3)
      {
        file << "# U, V, W, uu, vv, ww, uv, uw, vw, U_rolling, V_rolling, W_rolling, uu_rolling, vv_rolling, ww_rolling, uv_rolling, uw_rolling, vw_rolling\n";
      }
      const Uint global_nb_means = global_means.size();
      cf3_assert(global_rolling_means.size() == global_nb_means);
      cf3_assert(global_nb_means % stride == 0);
      const Uint global_nb_nodes = global_nb_means / stride;
      for(Uint i = 0; i != global_nb_nodes; ++i)
      {
        file << global_means[i*stride];
        for(Uint j = 1; j != stride; ++j)
          file << " " << global_means[i*stride+j];
        
        for(Uint j = 0; j != stride; ++j)
          file << " " << global_rolling_means[i*stride+j];
        file << "\n";
      }
      file.close();
    }
  }

  // Write the probe data
  const Uint nb_my_probes = m_probe_nodes.size();
  for(Uint my_probe_idx = 0; my_probe_idx != nb_my_probes; ++my_probe_idx)
  {
    const Uint node_begin = m_probe_nodes[my_probe_idx]*stride;
    boost::filesystem::fstream& file = *m_probe_files[my_probe_idx];
    for(Uint j = 0; j != stride; ++j)
    {
      if(j != 0)
        file << " ";
      file << boost::accumulators::mean(m_means[node_begin+j]);
    }
    
    for(Uint j = 0; j != stride; ++j)
    {
      file << " " << boost::accumulators::rolling_mean(m_rolling_means[node_begin+j]);
    }
    
    file << "\n";
  }
}

void TurbulenceStatistics::reset_statistics()
{
  const Uint nb_accs = (2.*m_dim + m_dim-1 + m_dim-2)*m_used_nodes.size();
  m_means.assign(nb_accs, MeanAccT());
  m_rolling_means.assign(nb_accs, RollingAccT(boost::accumulators::tag::rolling_window::window_size = options().value<Uint>("rolling_window")));
  m_count = 0;
}

void TurbulenceStatistics::add_probe(const RealVector& probe_location)
{
  m_probe_locations.push_back(probe_location);
}

void TurbulenceStatistics::trigger_option()
{
  m_options_changed = true;
}

void TurbulenceStatistics::signal_add_probe(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  std::vector<Real> probe_location = options.value< std::vector<Real> >("probe_location");
  add_probe(RealVector(RealVector::Map(probe_location.data(), probe_location.size())));
}

void TurbulenceStatistics::signature_add_probe(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  options.add("probe_location", std::vector<Real>()).pretty_name("Probe Location").description("Location to probe, i.e. write every timestep");
}

void TurbulenceStatistics::setup()
{
  if(!m_options_changed)
    return;
  m_options_changed = false;

  m_used_nodes.clear();

  Handle<mesh::Region> region = options().value< Handle<mesh::Region> >("region");
  if(is_null(region))
  {
    return;
  }

  const std::string var_name = options().value<std::string>("variable_name");
  m_field.reset();
  
  const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*region);
  Handle<mesh::Dictionary> dictionary;
  BOOST_FOREACH(const Handle<mesh::Dictionary>& dict, mesh.dictionaries())
  {
    BOOST_FOREACH(const Handle<mesh::Field>& field, dict->fields())
    {
      if(field->has_variable(var_name))
      {
        if(is_not_null(m_field))
          throw common::SetupError(FromHere(), "There are two fields that contain the variable " + var_name);
        
        m_field = field;
        dictionary = dict;
      }
    }
  }
  
  if(is_null(m_field))
    throw common::SetupError(FromHere(), "There is no field with the variable " + var_name);
  
  m_dim = mesh.dimension();
  if(m_dim == 1)
  {
    throw common::SetupError(FromHere(), "TurbulenceStatistics are not supported in 1D");
  }
  cf3_assert(m_field->var_length(var_name) == m_dim);
  m_field_offset = m_field->var_offset(var_name);

  const mesh::Field& coords = dictionary->coordinates();
  
  common::PE::Comm& comm = common::PE::Comm::instance();

  boost::shared_ptr< common::List<Uint> >used_nodes_list = mesh::build_used_nodes_list(*region, *dictionary, false, false);
  m_used_nodes.reserve(used_nodes_list->size());
  const int nb_probes = m_probe_locations.size();
  std::vector<int> my_probes_found(nb_probes, 0);
  m_probe_nodes.clear();
  m_probe_indices.clear();
  BOOST_FOREACH(const Uint node_idx, used_nodes_list->array())
  {
    if(dictionary->is_ghost(node_idx))
      continue;
    for(int probe_idx = 0; probe_idx != nb_probes; ++probe_idx)
    {
      if(m_probe_locations[probe_idx].size() != m_dim)
      {
        throw common::SetupError(FromHere(), "Probe coordinates of dimension " + common::to_str(m_probe_locations[probe_idx].size()) + " do not match dimension " + common::to_str(m_dim));
      }
      if(((RealVector::Map(&coords[node_idx][0], m_dim) - m_probe_locations[probe_idx]).array().abs() < 1e-10).all())
      {
        m_probe_nodes.push_back(m_used_nodes.size());
        m_probe_indices.push_back(probe_idx);
        my_probes_found[probe_idx] = 1;
      }
    }
    m_used_nodes.push_back(node_idx);
  }

  std::vector<int> global_probes_found(nb_probes);
  if(comm.is_active() && nb_probes > 0)
  {
    comm.all_reduce(common::PE::plus(), my_probes_found, global_probes_found);
  }
  else
  {
    global_probes_found = my_probes_found;
  }
  for(int probe_idx = 0; probe_idx != nb_probes; ++probe_idx)
  {
    if(global_probes_found[probe_idx] == 0)
      throw common::SetupError(FromHere(), "Probe " + common::to_str(probe_idx) + " has no matching node");
    if(global_probes_found[probe_idx] > 1)
      throw common::SetupError(FromHere(), "Probe " + common::to_str(probe_idx) + " was found on " + common::to_str(global_probes_found[probe_idx]) + " CPUs");
  }

  const common::URI original_uri = options().value<common::URI>("file");

  // Init probe files
  m_probe_files.clear();
  const Uint nb_my_probes = m_probe_nodes.size();
  for(Uint my_idx = 0; my_idx != nb_my_probes; ++my_idx)
  {
    const Uint probe_idx = m_probe_indices[my_idx];
    std::string probe_path = original_uri.path();
    if(boost::algorithm::contains(probe_path, "{iteration}"))
    {
      boost::algorithm::replace_all(probe_path, "{iteration}", "probe-" + common::to_str(probe_idx));
    }
    else
    {
      probe_path = (original_uri.base_path() / (original_uri.base_name() + "-probe-" + common::to_str(probe_idx) + original_uri.extension())).path();
    }
    m_probe_files.push_back(boost::make_shared<boost::filesystem::fstream>(probe_path, std::ios_base::out));
    boost::filesystem::fstream& file = *m_probe_files.back();
    if(!file)
      throw common::FileSystemError(FromHere(), "Failed to open file " + probe_path);

    file << "# Probe data for probe " << probe_idx << " at point " << m_probe_locations[probe_idx].transpose() << "\n";
    if(m_dim == 2)
    {
      file << "# U, V, uu, vv, uv, U_rolling, V_rolling, uu_rolling, vv_rolling, uv_rolling\n";
    }
    else if(m_dim == 3)
    {
      file << "# U, V, W, uu, vv, ww, uv, uw, vw, U_rolling, V_rolling, W_rolling, uu_rolling, vv_rolling, ww_rolling, uv_rolling, uw_rolling, vw_rolling\n";
    }
  }

  // Write out the coordinates
  std::string coords_path = original_uri.path();
  if(boost::algorithm::contains(coords_path, "{iteration}"))
  {
    boost::algorithm::replace_all(coords_path, "{iteration}", "coordinates");
  }
  else
  {
    coords_path = (original_uri.base_path() / (original_uri.base_name() + "-coordinates" + original_uri.extension())).path();
  }
  
  const std::vector<Real> used_coords = detail::gather_vector_field(m_used_nodes, coords);
  cf3_assert(used_coords.size() % m_dim == 0);
  
  if(comm.rank() == 0)
  {
    boost::filesystem::fstream file(coords_path, std::ios_base::out);
    if(!file)
      throw common::FileSystemError(FromHere(), "Failed to open file " + coords_path);
    file << "# x y";
    if(m_dim > 2)
      file << " z";
    file << "\n";
    
    const Uint nb_coords = used_coords.size();
    for(Uint i = 0; i != nb_coords;)
    {
      file << used_coords[i];
      ++i;
      if(i % m_dim != 0)
        file << " ";
      else
        file << "\n";
    }
    file.close();
  }

  reset_statistics();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

