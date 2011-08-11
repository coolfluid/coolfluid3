// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/BasicExceptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;
using namespace Physics;

void create_fields(CMesh& mesh, PhysModel& physical_model)
{
  typedef std::map<std::string, std::string> FieldsT;
  FieldsT fields;
  physical_model.variable_manager().field_specification(fields);

  for(FieldsT::const_iterator it = fields.begin(); it != fields.end(); ++it)
  {
    const std::string& field_name = it->first;
    const std::string& var_spec = it->second;

    Component::Ptr field_comp = mesh.get_child_ptr(field_name);
    if(is_not_null(field_comp))
    {
      CField::Ptr field = boost::dynamic_pointer_cast<CField>(field_comp);
      if(!field)
        throw SetupError(FromHere(), "Adding fields into " + mesh.uri().string() + ": Component with name " + field_name + " exists, but it is not a field");

      // Check if the existing field is compatible with the new one
      std::stringstream existing_spec;
      for(Uint i = 0; i != field->nb_vars(); ++i)
      {
        existing_spec << (i > 0 ? "," : "") << field->var_name(i) << "[" << field->var_type(i) << "]";
      }
      if(existing_spec.str() != var_spec)
        throw SetupError(FromHere(), "Adding fields into " + mesh.uri().string() + ": Field with name " + field_name + " exists, but it is incompatible: old spec " + existing_spec.str() + " != new spec " + var_spec);
    }
    else
    {
      mesh.create_field(field_name, CF::Mesh::CField::Basis::POINT_BASED, "space[0]", var_spec);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
