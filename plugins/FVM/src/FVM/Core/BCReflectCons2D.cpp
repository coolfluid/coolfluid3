// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementData.hpp"
#include "FVM/Core/BCReflectCons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCReflectCons2D, BC, LibCore > BCReflectCons2D_Builder;
Common::ComponentBuilder < BCReflectCons2D, CAction, LibCore > BCReflectCons2D_CAction_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BCReflectCons2D::BCReflectCons2D ( const std::string& name ) :
  BC(name),
  m_connected_solution("solution_view"),
  m_face_normal("face_normal")
{
  mark_basic();
  // options
  m_options.add_option(OptionURI::create("solution","Solution","Cell based solution","cpath:/",URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCReflectCons2D::config_solution,   this ) );

  m_options.add_option(OptionURI::create(Mesh::Tags::normal(),"Face Normal","Unit normal to the face, outward from left cell", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCReflectCons2D::config_normal,   this ) );

  m_options["Elements"].attach_trigger ( boost::bind ( &BCReflectCons2D::trigger_elements,   this ) );

}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons2D::config_solution()
{
  URI uri;  option("solution").put_value(uri);
  CField::Ptr comp = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) ) throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons2D::config_normal()
{
  URI uri;  option(Mesh::Tags::normal()).put_value(uri);
  CField& comp = Common::Core::instance().root().access_component(uri).as_type<CField>();
  m_face_normal.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCReflectCons2D::trigger_elements()
{
  m_can_start_loop = m_connected_solution.set_elements(elements());
  m_can_start_loop &=  m_face_normal.set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void BCReflectCons2D::execute()
{
  cf_assert(m_face_normal.size());
  std::vector<CTable<Real>::Row> solution = m_connected_solution[idx()];
  normal = to_vector(m_face_normal[idx()]);

  U << solution[INNER][1]/solution[INNER][0],
       solution[INNER][2]/solution[INNER][0];

  U_n = (U.dot(normal)) *normal;// normal velocity
  U_t = U - U_n;         // tangential velocity

  U = -U_n + U_t;  // switched sign of normal velocity

  // Change value in ghost cell
  solution[GHOST][0] = solution[INNER][0];
  solution[GHOST][1] = U[XX]*solution[INNER][0];
  solution[GHOST][2] = U[YY]*solution[INNER][0];
  solution[GHOST][3] = solution[INNER][3];

}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

