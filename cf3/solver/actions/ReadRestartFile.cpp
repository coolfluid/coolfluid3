// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/BinaryDataReader.hpp"

#include "common/XML/FileOperations.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Tags.hpp"
#include "solver/Time.hpp"

#include "solver/actions/ReadRestartFile.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ReadRestartFile, common::Action, LibActions > ReadRestartFile_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ReadRestartFile::ReadRestartFile ( const std::string& name ) :
  common::Action(name)
{  
  options().add("mesh", Handle<mesh::Mesh>())
    .pretty_name("Mesh")
    .description("Mesh containing the fields")
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name for the input file")
    .mark_basic();
    
  options().add(solver::Tags::time(), Handle<solver::Time>())
    .pretty_name("Time")
    .description("Time component, filled with the timing info from the file")
    .mark_basic();

  options().add("read_time_settings", true)
    .pretty_name("Read  Time Settings")
    .description("Use the time step from the restart file")
    .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void ReadRestartFile::execute()
{
  Handle<mesh::Mesh> mesh = options().value< Handle<mesh::Mesh> >("mesh");
  if(is_null(mesh))
    throw common::SetupError(FromHere(), "Option mesh is not configured");

  Handle<Time> time = options().value< Handle<Time> >(solver::Tags::time());
  if(is_null(time))
    throw common::SetupError(FromHere(), "Time component not configured");
  
  const common::URI filepath = options().value<common::URI>("file");
  boost::shared_ptr<common::XML::XmlNode> input_file = common::XML::parse_file(filepath);

  common::XML::XmlNode restart_node(input_file->content->first_node("restart"));
  if(!restart_node.is_valid())
    throw common::FileFormatError(FromHere(), "File  " + filepath.path() + " has no restart node");
  
  if(options().value<bool>("read_time_settings"))
  {
    time->options().set("time_step", common::from_str<Real>(restart_node.attribute_value("time_step")));
    time->options().set("current_time", common::from_str<Real>(restart_node.attribute_value("current_time")));
    time->options().set("iteration", common::from_str<Uint>(restart_node.attribute_value("iteration")));
  }

  if(common::from_str<Uint>(restart_node.attribute_value("version")) != 1)
    throw common::FileFormatError(FromHere(), "File  " + filepath.path() + " has unsupported version");

  common::PE::Comm& comm = common::PE::Comm::instance();
  if(common::from_str<Uint>(restart_node.attribute_value("nb_procs")) != comm.size())
    throw common::SetupError(FromHere(), "File  " + filepath.path() + " was made for " + restart_node.attribute_value("nb_procs") + " CPUs, but we are loading on " + common::to_str(comm.size()) + " CPUs");

  boost::shared_ptr<common::BinaryDataReader> data_reader = common::allocate_component<common::BinaryDataReader>("DataReader");
  data_reader->options().set("file", common::URI(restart_node.attribute_value("binary_file")));

  common::XML::XmlNode field_node = restart_node.content->first_node("field");
  for(; field_node.is_valid(); field_node.content = field_node.content->next_sibling("field"))
  {
    Handle<mesh::Field> field(mesh->access_component(common::URI(field_node.attribute_value("path"), common::URI::Scheme::CPATH)));
    if(is_null(field))
      throw common::SetupError(FromHere(), "Field " + field_node.attribute_value("path") + " was not found in mesh " + mesh->uri().path());

    data_reader->read_table(*field, common::from_str<Uint>(field_node.attribute_value("index")));
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

