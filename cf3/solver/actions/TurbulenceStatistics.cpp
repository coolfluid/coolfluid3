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
#include "common/PropertyList.hpp"
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
  inline void update_mean(Real& mean, const Real new_value, const Uint count)
  {
    if(count != 0)
      mean = (mean * static_cast<Real>(count) + new_value) / static_cast<Real>(count+1);
    else
      mean = new_value;
  }
}

TurbulenceStatistics::TurbulenceStatistics ( const std::string& name ) :
  common::Action(name),
  m_rolling_size(10),
  m_options_changed(true),
  m_dim(0),
  m_velocity_field_offset(0),
  m_pressure_field_offset(0),
  m_count(0)
{
  options().add("velocity_variable_name", "Velocity")
    .pretty_name("Velocity Variable Name")
    .description("The velocity field for which turbulent statistics are to be calculated")
    .attach_trigger(boost::bind(&TurbulenceStatistics::trigger_option, this))
    .mark_basic();
    
  options().add("pressure_variable_name", "Pressure")
    .pretty_name("Pressure Variable Name")
    .description("The pressure field for which turbulent statistics are to be calculated")
    .attach_trigger(boost::bind(&TurbulenceStatistics::trigger_option, this))
    .mark_basic();

  options().add("region", Handle<mesh::Region>())
    .pretty_name("Region")
    .description("Region to loop over")
    .attach_trigger(boost::bind(&TurbulenceStatistics::trigger_option, this))
    .mark_basic();

  options().add("rolling_window", 10u)
    .pretty_name("Rolling window")
    .description("Window size for the rolling averages in probes")
    .attach_trigger(boost::bind(&TurbulenceStatistics::reset_statistics, this))
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("Base file name for the probe output files")
    .mark_basic();

  options().add("count", m_count)
    .pretty_name("Count")
    .description("Number of averages made")
    .link_to(&m_count);

  regist_signal( "add_probe" )
    .connect( boost::bind( &TurbulenceStatistics::signal_add_probe, this, _1 ) )
    .description("Add a probe at the given location, logging to its own file")
    .pretty_name("Add Probe")
    .signature( boost::bind( &TurbulenceStatistics::signature_add_probe, this, _1));

  regist_signal( "setup" )
    .connect( boost::bind( &TurbulenceStatistics::signal_setup, this, _1 ) )
    .description("Set up the internal counters and field for the statistics")
    .pretty_name("Setup");
  
  properties().add("restart_field_tags", std::vector<std::string>(1, "turbulence_statistics"));
}

void TurbulenceStatistics::execute()
{
  setup();
  common::PE::Comm& comm = common::PE::Comm::instance();

  const Uint nb_nodes = m_used_nodes->size();
  const common::List<Uint>::ListT& used_nodes_list = m_used_nodes->array();
  const mesh::Field::ArrayT& velocity_array = m_velocity_field->array();
  const mesh::Field::ArrayT& pressure_array = m_pressure_field->array();
  mesh::Field::ArrayT& means_array = m_statistics_field->array();
  const Uint stride = 2.*m_dim + m_dim-1 + m_dim-2;
  const Uint nb_my_probes = m_probe_nodes.size();

  if(m_dim == 2)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const mesh::Field::ConstRow velocity = velocity_array[used_nodes_list[i]];
      mesh::Field::Row means = means_array[used_nodes_list[i]];
      const Real u = velocity[XX+m_velocity_field_offset]; const Real v = velocity[YY+m_velocity_field_offset];
      const Real p = pressure_array[used_nodes_list[i]][m_pressure_field_offset];
    
      detail::update_mean(means[0], u, m_count);
      detail::update_mean(means[1], v, m_count);
      detail::update_mean(means[2], u*u, m_count);
      detail::update_mean(means[3], v*v, m_count);
      detail::update_mean(means[4], u*v, m_count);
      detail::update_mean(means[5], p, m_count);
      detail::update_mean(means[6], p*p, m_count);
    }

    for(Uint my_probe_idx = 0; my_probe_idx != nb_my_probes; ++my_probe_idx)
    {
      const Uint probe_begin = my_probe_idx*stride;
      const mesh::Field::ConstRow velocity = velocity_array[m_probe_nodes[my_probe_idx]];
      const Real u = velocity[XX+m_velocity_field_offset]; const Real v = velocity[YY+m_velocity_field_offset];

      m_means[probe_begin  ](u);
      m_means[probe_begin+1](v);
      m_means[probe_begin+2](u*u);
      m_means[probe_begin+3](v*v);
      m_means[probe_begin+4](u*v);

      m_rolling_means[probe_begin  ](u);
      m_rolling_means[probe_begin+1](v);
      m_rolling_means[probe_begin+2](u*u);
      m_rolling_means[probe_begin+3](v*v);
      m_rolling_means[probe_begin+4](u*v);
    }
  }
  else if(m_dim == 3)
  {
    for(Uint i = 0; i != nb_nodes; ++i)
    {
      const mesh::Field::ConstRow velocity = velocity_array[used_nodes_list[i]];
      mesh::Field::Row means = means_array[used_nodes_list[i]];
      const Real u = velocity[XX+m_velocity_field_offset]; const Real v = velocity[YY+m_velocity_field_offset]; const Real w = velocity[ZZ+m_velocity_field_offset];
      const Real p = pressure_array[used_nodes_list[i]][m_pressure_field_offset];
      
      detail::update_mean(means[0], u, m_count);
      detail::update_mean(means[1], v, m_count);
      detail::update_mean(means[2], w, m_count);
      detail::update_mean(means[3], u*u, m_count);
      detail::update_mean(means[4], v*v, m_count);
      detail::update_mean(means[5], w*w, m_count);
      detail::update_mean(means[6], u*v, m_count);
      detail::update_mean(means[7], u*w, m_count);
      detail::update_mean(means[8], v*w, m_count);
      detail::update_mean(means[9], p, m_count);
      detail::update_mean(means[10], p*p, m_count);
    }

    for(Uint my_probe_idx = 0; my_probe_idx != nb_my_probes; ++my_probe_idx)
    {
      const Uint probe_begin = my_probe_idx*stride;
      const mesh::Field::ConstRow velocity = velocity_array[m_probe_nodes[my_probe_idx]];
      const Real u = velocity[XX+m_velocity_field_offset]; const Real v = velocity[YY+m_velocity_field_offset]; const Real w = velocity[ZZ+m_velocity_field_offset];

      m_means[probe_begin  ](u);
      m_means[probe_begin+1](v);
      m_means[probe_begin+2](w);
      m_means[probe_begin+3](u*u);
      m_means[probe_begin+4](v*v);
      m_means[probe_begin+5](w*w);
      m_means[probe_begin+6](u*v);
      m_means[probe_begin+7](u*w);
      m_means[probe_begin+8](v*w);

      m_rolling_means[probe_begin  ](u);
      m_rolling_means[probe_begin+1](v);
      m_rolling_means[probe_begin+2](w);
      m_rolling_means[probe_begin+3](u*u);
      m_rolling_means[probe_begin+4](v*v);
      m_rolling_means[probe_begin+5](w*w);
      m_rolling_means[probe_begin+6](u*v);
      m_rolling_means[probe_begin+7](u*w);
      m_rolling_means[probe_begin+8](v*w);
    }
  }

  // Write the probe data
  for(Uint my_probe_idx = 0; my_probe_idx != nb_my_probes; ++my_probe_idx)
  {
    const Uint probe_begin = my_probe_idx*stride;
    const Uint probe_end = probe_begin + stride;
    boost::filesystem::fstream& file = *m_probe_files[my_probe_idx];
    for(Uint j = probe_begin; j != probe_end; ++j)
    {
      if(j != 0)
        file << " ";
      file << boost::accumulators::mean(m_means[j]);
    }
    
    for(Uint j = 0; j != stride; ++j)
    {
      file << " " << boost::accumulators::rolling_mean(m_rolling_means[j]);
    }
    
    file << "\n";
  }

  options().set("count", m_count+1u);
}

void TurbulenceStatistics::reset_statistics()
{
  const Uint nb_accs = (2.*m_dim + m_dim-1 + m_dim-2)*m_probe_nodes.size();
  m_means.assign(nb_accs, MeanAccT());
  m_rolling_means.assign(nb_accs, RollingAccT(boost::accumulators::tag::rolling_window::window_size = options().value<Uint>("rolling_window")));
  options().set("count", 0u);
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

void TurbulenceStatistics::signal_setup(common::SignalArgs& args)
{
  setup();
}

void TurbulenceStatistics::setup()
{
  if(!m_options_changed)
    return;
  m_options_changed = false;

  m_used_nodes.reset();

  Handle<mesh::Region> region = options().value< Handle<mesh::Region> >("region");
  if(is_null(region))
  {
    return;
  }

  const std::string velocity_var_name = options().value<std::string>("velocity_variable_name");
  const std::string pressure_var_name = options().value<std::string>("pressure_variable_name");
  m_velocity_field.reset();
  m_pressure_field.reset();
  
  const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*region);
  Handle<mesh::Dictionary> dictionary;
  BOOST_FOREACH(const Handle<mesh::Dictionary>& dict, mesh.dictionaries())
  {
    BOOST_FOREACH(const Handle<mesh::Field>& field, dict->fields())
    {
      if(field->has_variable( velocity_var_name ))
      {
        if(is_not_null(m_velocity_field))
          throw common::SetupError(FromHere(), "There are two fields that contain the variable " + velocity_var_name );
        
        m_velocity_field = field;
        dictionary = dict;
      }
    }
  }
  
  if(is_null(m_velocity_field))
    throw common::SetupError(FromHere(), "There is no field with the variable " + velocity_var_name );
  
  m_dim = mesh.dimension();
  if(m_dim == 1)
  {
    throw common::SetupError(FromHere(), "TurbulenceStatistics are not supported in 1D");
  }
  cf3_assert(m_velocity_field->var_length(velocity_var_name) == m_dim);
  m_velocity_field_offset = m_velocity_field->var_offset( velocity_var_name );
  
  BOOST_FOREACH(const Handle<mesh::Field>& field, dictionary->fields())
  {
    if(field->has_variable( pressure_var_name ))
    { 
      m_pressure_field = field;
    }
  }
  
  if(is_null(m_pressure_field))
    throw common::SetupError(FromHere(), "There is no field with the variable " + pressure_var_name );
  
  m_pressure_field_offset = m_pressure_field->var_offset( pressure_var_name );

  const mesh::Field& coords = dictionary->coordinates();
  
  common::PE::Comm& comm = common::PE::Comm::instance();

  m_used_nodes = mesh::build_used_nodes_list(*region, *dictionary, true, false);
  const int nb_probes = m_probe_locations.size();
  std::vector<int> my_probes_found(nb_probes, 0);
  m_probe_nodes.clear();
  m_probe_indices.clear();
  const common::List<Uint>::ListT& used_nodes_list = m_used_nodes->array();
  BOOST_FOREACH(const Uint node_idx, used_nodes_list)
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
        m_probe_nodes.push_back(node_idx);
        m_probe_indices.push_back(probe_idx);
        my_probes_found[probe_idx] = 1;
      }
    }
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
    std::string probe_path = (original_uri.base_path() / (original_uri.base_name() + "-probe-" + common::to_str(probe_idx) + original_uri.extension())).path();

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

  // Create a field for the statistics data
  m_statistics_field = Handle<mesh::Field>(dictionary->get_child("turbulence_statistics"));
  if(is_null(m_statistics_field))
  {
    if(m_dim == 2)
    {
      m_statistics_field = dictionary->create_field("turbulence_statistics", "V[vector],uu,vv,uv,p,pp").handle<mesh::Field>();
    }
    else if(m_dim == 3)
    {
      m_statistics_field = dictionary->create_field("turbulence_statistics", "V[vector],uu,vv,ww,uv,uw,vw,p,pp").handle<mesh::Field>();
    }
    m_statistics_field->add_tag("turbulence_statistics");
  }

  // Reset statistics without changing m_count
  const Uint nb_accs = (2.*m_dim + m_dim-1 + m_dim-2)*m_probe_nodes.size();
  m_means.assign(nb_accs, MeanAccT());
  m_rolling_means.assign(nb_accs, RollingAccT(boost::accumulators::tag::rolling_window::window_size = options().value<Uint>("rolling_window")));
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

