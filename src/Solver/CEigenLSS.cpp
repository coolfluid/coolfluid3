// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CField.hpp"

#include "CEigenLSS.hpp"

namespace CF {
namespace Solver {

CF::Common::ComponentBuilder < CEigenLSS, Common::Component, LibSolver > aCeigenLSS_Builder;

CEigenLSS::CEigenLSS ( const std::string& name ) : Component ( name )
{
  Common::Option::Ptr option = m_properties.add_option<Common::OptionURI>("SolutionField", "Path to the field that will store the solution", "");
  boost::dynamic_pointer_cast<Common::OptionURI>(option)->supported_protocol(CF::Common::URI::Scheme::CPATH);
}

void CEigenLSS::resize ( Uint nb_dofs )
{
  m_system_matrix.resize(nb_dofs, nb_dofs);
  m_rhs.resize(nb_dofs);
}

Uint CEigenLSS::size() const
{
  return m_system_matrix.cols();
}

RealMatrix& CEigenLSS::matrix()
{
  return m_system_matrix;
}

RealVector& CEigenLSS::rhs()
{
  return m_rhs;
}

void CEigenLSS::solve()
{
  Mesh::CField::Ptr output_field = look_component<Mesh::CField>(property("SolutionField").value_str());
  cf_assert(output_field);

  Mesh::CTable<Real>& output_data = Common::find_component_with_filter< Mesh::CTable<Real> >(*output_field, Common::IsComponentTag("field_data"));
  const Uint row_size = output_data.row_size();

  const RealVector solution = matrix().colPivHouseholderQr().solve(rhs());

  for(Uint i = 0; i != size(); ++i)
  {
    const Uint row = i / row_size;
    const Uint col = i % row_size;
    output_data[row][col] = solution[i];
  }
}

} // Solver
} // CF
