// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"
#include "common/BinaryDataWriter.hpp"
#include "common/XML/FileOperations.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Tags.hpp"
#include "solver/Time.hpp"

#include "solver/actions/WriteRestartFile.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WriteRestartFile, common::Action, LibActions > WriteRestartFile_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WriteRestartFile::WriteRestartFile ( const std::string& name ) :
  common::Action(name)
{
  options().add("fields", std::vector< Handle<mesh::Field> >())
    .pretty_name("Fields")
    .description("Fields to store for restart")
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name for the output file")
    .mark_basic();
    
  options().add(solver::Tags::time(), Handle<solver::Time>())
    .pretty_name("Time")
    .description("Time component, used to extract timing and iteration information")
    .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void WriteRestartFile::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  
  std::vector< Handle<mesh::Field> > fields = options().value< std::vector< Handle<mesh::Field> > >("fields");
  if(fields.empty())
    throw common::SetupError(FromHere(), "No fields configured");
  
  Handle<Time> time = options().value< Handle<Time> >(solver::Tags::time());
  if(is_null(time))
    throw common::SetupError(FromHere(), "Time component not configured");
  
  // Enssure each field is from the same mesh
  Handle<mesh::Mesh> mesh;
  BOOST_FOREACH(const Handle<mesh::Field>& field, fields)
  {
    Handle<mesh::Mesh> parent_mesh = common::find_parent_component_ptr<mesh::Mesh>(*field);
    if(parent_mesh != mesh && is_not_null(mesh))
      throw common::SetupError(FromHere(), "Fields do not belong to the same mesh");
    mesh = parent_mesh;
  }
  cf3_assert(is_not_null(mesh));
  
  const common::URI out_file_path = options().value<common::URI>("file");
  const common::URI binfile = out_file_path.base_path() / (out_file_path.base_name() + ".cfbinxml");
  boost::shared_ptr<common::BinaryDataWriter> data_writer = common::allocate_component<common::BinaryDataWriter>("DataWriter");
  data_writer->options().set("file", binfile);
  
  common::XML::XmlDoc xml_doc("1.0", "ISO-8859-1");
  common::XML::XmlNode restart_node = xml_doc.add_node("restart");
  restart_node.set_attribute("version", "1");
  restart_node.set_attribute("binary_file", binfile.path());
  restart_node.set_attribute("nb_procs", common::to_str(comm.size()));
  restart_node.set_attribute("current_time", common::to_str(time->current_time()));
  restart_node.set_attribute("time_step", common::to_str(time->dt()));
  restart_node.set_attribute("iteration", common::to_str(time->iter()));
  
  const std::string base_path = mesh->uri().path() + "/";
  
  BOOST_FOREACH(const Handle<mesh::Field>& field, fields)
  {
    common::XML::XmlNode field_node = restart_node.add_node("field");
    std::string relative_path = field->uri().path();
    boost::replace_first(relative_path, base_path, "");
    cf3_assert(relative_path.size() == field->uri().path().size() - base_path.size());
    field_node.set_attribute("path", relative_path);
    field_node.set_attribute("index", common::to_str(data_writer->append_data(*field)));
  }

  if(comm.rank() == 0)
    common::XML::to_file(xml_doc, out_file_path);
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

