// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CCells.hpp"

#include "FVM/Core/BCReflectCons1D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCReflectCons1D, BC, LibCore > BCReflectCons1D_Builder;
Common::ComponentBuilder < BCReflectCons1D, CAction, LibCore > BCReflectCons1D_CAction_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BCReflectCons1D::BCReflectCons1D ( const std::string& name ) : 
  BC(name),
  m_connected_solution("solution_view"),
  m_face_normal("face_normal")
{
  mark_basic();
  // options
  m_properties.add_option(OptionURI::create("Solution","Cell based solution","cpath:/",URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCReflectCons1D::config_solution,   this ) )
    ->add_tag("solution");
    
  m_properties.add_option(OptionURI::create("FaceNormal","Unit normal to the face, outward from left cell", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCReflectCons1D::config_normal,   this ) )
    ->add_tag(Mesh::Tags::normal());

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BCReflectCons1D::trigger_elements,   this ) );
  
}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons1D::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField::Ptr comp = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) ) throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons1D::config_normal()
{
  URI uri;  property("FaceNormal").put_value(uri);
  CField& comp = Common::Core::instance().root().access_component(uri).as_type<CField>();
  m_face_normal.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons1D::trigger_elements()
{
  m_can_start_loop = m_connected_solution.set_elements(elements());
  //m_can_start_loop &=  m_face_normal.set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void BCReflectCons1D::execute()
{
  // CFinfo << "executing " << full_path().path() << CFendl;
  // CFinfo << "        on face " << elements().full_path().path() << "["<<idx()<<"]" << CFendl;
  // CFaceCellConnectivity& f2c = find_component<CFaceCellConnectivity>(elements());
  // CTable<Uint>::ConstRow elems = f2c.elements(idx());
  // CCells::Ptr cells;
  // Uint cell_idx(0);
  // boost::tie(cells,cell_idx) = f2c.element_location(elems[INNER]);
  // CFinfo << "   inner: " << cells->full_path().path() << "["<<cell_idx<<"]" << CFendl;  
  // boost::tie(cells,cell_idx) = f2c.element_location(elems[GHOST]);
  // CFinfo << "   ghost: " << cells->full_path().path() << "["<<cell_idx<<"]" << CFendl;  
  
  // Change value in field
  // problem: GHOST does not exist yet.
  std::vector<CTable<Real>::Row> solution = m_connected_solution[idx()];
  solution[GHOST][0] =  solution[INNER][0];
  solution[GHOST][1] = -solution[INNER][1];
  solution[GHOST][2] =  solution[INNER][2];
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

