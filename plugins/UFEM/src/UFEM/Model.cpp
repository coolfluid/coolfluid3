// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "Model.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;
using namespace Solver;

struct Model::Implementation
{
  Implementation(Component& comp) :
    m_component(comp),
    m_mesh(m_component.create_static_component_ptr<CMesh>("Mesh")),
    m_physical_model(m_component.create_static_component_ptr<CPhysicalModel>("PhysicalModel"))
  {
    m_physical_model.lock()->configure_option("mesh", m_mesh);
    m_component.options().add_option< OptionURI >("input_file", "Input File", "Path to the mesh that is read when the \"Read Mesh\" signal is called")
      ->link_to(&m_input_file);
    m_component.options().add_option< OptionURI >("output_file", "Output File", "Path to the mesh that is written when the \"Write Mesh\" signal is called")
      ->link_to(&m_output_file);
  }
  
  Component& m_component;
  boost::weak_ptr<CMesh> m_mesh;
  boost::weak_ptr<CPhysicalModel> m_physical_model;
  
  boost::weak_ptr<LoadMesh> m_load_mesh;
  boost::weak_ptr<WriteMesh> m_write_mesh;
  
  URI m_input_file;
  URI m_output_file;
};

Model::Model(const std::string& name) :
  CProtoActionDirector(name),
  m_implementation( new Implementation(*this) )
{
  configure_option("physical_model", m_implementation->m_physical_model);
  configure_option("region", m_implementation->m_mesh.lock()->topology().as_ptr<CRegion>());
}

Model::~Model()
{
}

void Model::execute()
{
  // Create the fields, if this was not done before
  m_implementation->m_physical_model.lock()->create_fields();
  CF::Common::CActionDirector::execute();
}

void Model::signal_read_mesh(CF::Common::SignalArgs& node)
{
  if(m_implementation->m_load_mesh.expired()) // created on-demand
    m_implementation->m_load_mesh = create_static_component_ptr<LoadMesh>("MeshLoader");
  
  m_implementation->m_load_mesh.lock()->load_mesh_into(m_implementation->m_input_file, *m_implementation->m_mesh.lock());
}


void Model::signal_write_mesh(CF::Common::SignalArgs& node)
{
  if(m_implementation->m_write_mesh.expired()) // created on-demand
    m_implementation->m_write_mesh = create_static_component_ptr<WriteMesh>("MeshWriter");
  
  std::vector<URI> state_fields;
  physical_model().state_fields(state_fields);
  m_implementation->m_write_mesh.lock()->write_mesh(*m_implementation->m_mesh.lock(), m_implementation->m_output_file, state_fields);
}


} // UFEM
} // CF
