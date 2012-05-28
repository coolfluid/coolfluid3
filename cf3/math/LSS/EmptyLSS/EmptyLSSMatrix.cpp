// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "common/Builder.hpp"
#include "common/PropertyList.hpp"

#include "EmptyLSSMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

EmptyLSSMatrix::EmptyLSSMatrix ( const std::string& name ) :
  LSS::Matrix(name),
  m_blockcol_size(0),
  m_blockrow_size(0),
  m_neq(0),
  m_is_created(false)
{
  properties().add("vector_type", std::string("cf3.math.LSS.EmptyLSSVector"));
}


common::ComponentBuilder < LSS::EmptyLSSMatrix, LSS::Matrix, LSS::LibLSS > EmptyLSSMatrix_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3
