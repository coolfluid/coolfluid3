// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/OptionURI.hpp"
#include "mesh/CField.hpp"
#include "mesh/CSpace.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/CEntities.hpp"

#include "SFDM/ComputeJacobianDeterminant.hpp"
#include "SFDM/ShapeFunction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeJacobianDeterminant, Solver::Actions::CLoopOperation, LibSFDM > ComputeJacobianDeterminant_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeJacobianDeterminant::ComputeJacobianDeterminant ( const std::string& name ) :
  Solver::Actions::CLoopOperation(name)
{
  // options
  m_options.add_option(OptionURI::create("jacobian_determinant", URI("cpath:"), URI::Scheme::CPATH))
    ->description("Field storing the Jacobian Determinant")
    ->pretty_name("Jacobian Determinant")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeJacobianDeterminant::config_jacobian_determinant,   this ) )
    ->add_tag("jacobian_determinant");

  m_options["Elements"].attach_trigger ( boost::bind ( &ComputeJacobianDeterminant::trigger_elements,   this ) );

  m_jacobian_determinant = create_static_component_ptr<CMultiStateFieldView>("jacobian_determinant_view");

}

////////////////////////////////////////////////////////////////////////////////

void ComputeJacobianDeterminant::config_jacobian_determinant()
{
  URI uri;
  option("jacobian_determinant").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_jacobian_determinant->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeJacobianDeterminant::trigger_elements()
{
  m_can_start_loop = m_jacobian_determinant->set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeJacobianDeterminant::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex
  Real jacobian_determinant;
  const ShapeFunction& shape_func = m_jacobian_determinant->space().shape_function().as_type<ShapeFunction>();
  const ElementType&   geometry   = elements().element_type();

  CMultiStateFieldView::View jacobian_determinant_data = (*m_jacobian_determinant)[idx()];

  RealMatrix geometry_coords  = elements().get_coordinates( idx() );
  RealMatrix local_coords = shape_func.local_coordinates();
  for (Uint point=0; point<shape_func.nb_nodes(); ++point)
  {
    jacobian_determinant = geometry.jacobian_determinant(local_coords.row(point),geometry_coords);
    cf_assert( jacobian_determinant > 0. );
    jacobian_determinant_data[point][0] = jacobian_determinant;
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

