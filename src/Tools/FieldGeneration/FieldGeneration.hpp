// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_FieldGeneration_Tools_hpp
#define CF_Tools_FieldGeneration_Tools_hpp

#include "Common/Component.hpp"

#include "Tools/FieldGeneration/LibFieldGeneration.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Mesh { class CMesh; }
namespace Tools {
namespace FieldGeneration {

////////////////////////////////////////////////////////////////////////////////

/// Creates a scalar field with a constant value
void create_constant_scalar_field(Mesh::CMesh& mesh, const std::string& field_name, const std::string& var_name, const Real value);

class FieldGeneration_API FieldGenerator : public Common::Component
{
public:
  typedef boost::shared_ptr<FieldGenerator> Ptr;
  typedef boost::shared_ptr<FieldGenerator const> ConstPtr;
  
  FieldGenerator(const std::string& name);
  
  static std::string type_name () { return "FieldGenerator"; }
  
private:
  void signal_update(Common::SignalArgs& node);
};

////////////////////////////////////////////////////////////////////////////////

} // FieldGeneration
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_FieldGeneration_Tools_hpp

