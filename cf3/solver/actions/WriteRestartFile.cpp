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
#include "common/BinaryDataWriter.hpp"
#include "common/XML/FileOperations.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

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
  
  options().add("mesh", Handle<mesh::Mesh>())
    .pretty_name("Mesh")
    .description("Mesh containing the fields")
    .mark_basic();
    
  options().add("file", common::URI())
    .pretty_name("File")
    .description("File name for the output file")
    .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void WriteRestartFile::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  
  Handle<mesh::Mesh> mesh = options().value< Handle<mesh::Mesh> >("mesh");
  if(is_null(mesh))
    throw common::SetupError(FromHere(), "Option mesh is not configured");
  
  std::vector< Handle<mesh::Field> > fields = options().value< std::vector< Handle<mesh::Field> > >("fields");
  if(fields.empty())
    throw common::SetupError(FromHere(), "No fields configured");
  
  const common::URI out_file_path = options().value<common::URI>("file");
  const common::URI binfile = out_file_path.base_path() / (out_file_path.base_name() + ".cfbinxml");
  boost::shared_ptr<common::BinaryDataWriter> data_writer = common::allocate_component<common::BinaryDataWriter>("DataWriter");
  data_writer->options().set("file", binfile);
  
  common::XML::XmlDoc xml_doc("1.0", "ISO-8859-1");
  common::XML::XmlNode restart_node = xml_doc.add_node("restart");
  restart_node.set_attribute("version", "1");
  restart_node.set_attribute("binary_file", binfile.path());
  restart_node.set_attribute("nb_procs", common::to_str(comm.size()));
  
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

