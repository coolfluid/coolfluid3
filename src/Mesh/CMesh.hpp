// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMesh_hpp
#define CF_Mesh_CMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
  namespace Common {
    class CLink;
  }
namespace Mesh {

  class CNodes;
  class CRegion;
  
////////////////////////////////////////////////////////////////////////////////

/// Mesh component class
/// Mesh now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API CMesh : public Common::Component {
public: // typedefs

  typedef boost::shared_ptr<CMesh> Ptr;
  typedef boost::shared_ptr<CMesh const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~CMesh();

  /// Get the class name
  static std::string type_name () { return "CMesh"; }

  // functions specific to the CMesh component
  
  /// @return the geometry topology
  const CRegion& topology() const { return *m_topology; }
  
  /// @return the geometry topology
  CRegion& topology() { return *m_topology; }
  
  /// create a field with a given support
  /// @param name of the field
  CField& create_field( const std::string& name , CRegion& support, const Uint size, const CField::DataBasis basis);
  
  /// create a field with the default support being the full mesh geometry
  CField& create_field( const std::string& name , const Uint size, const CField::DataBasis basis);
	
	/// create a field with a given support
  /// @param name of the field
  CField& create_field( const std::string& name , CRegion& support, const std::vector<std::string>& variables, const CField::DataBasis basis);
  
  /// create a field with the default support being the full mesh geometry
  CField& create_field( const std::string& name , const std::vector<std::string>& variables, const CField::DataBasis basis);
  
  /// Create a field
  /// @param name Name for the field component
  /// @param base String representing the storage method. See CField2::DataBasis for valid values
  /// @param variables Either a comma-separated string of the form variable_name[size], or "scalar_same_name" (the default), which indicates the field holds a single scalar
  CField2& create_field2( const std::string& name , const std::string& base, const std::string& variables = "scalar_same_name");
  
  /// Create a field
  /// @param name Name for the field component
  /// @param base Storage method
  /// @param variable_names The names of the variables to add
  /// @param variable_types The types of the variables to add
  CField2& create_field2( const std::string& name , const CField2::DataBasis::Type base, const std::vector<std::string>& variable_names, const std::vector<CField2::VarType> variable_types);

  CField2& create_field2( const std::string& name , CField2& based_on_field);
  
  /// Create a field containing a single scalar
  CField2& create_scalar_field( const std::string& field_name, const std::string& variable_name, const CField2::DataBasis::Type base);
  
  CField2& create_scalar_field( const std::string& name , CField2& based_on_field);
  
  /// @return the field with given name
  const CField& field(const std::string& name) const;
  
  /// @return the field with given name
  CField& field(const std::string& name);
  
  void update_statistics();
  
  /// @return the nodes of the mesh , modifiable access
  CNodes& nodes();
  
  /// @return the nodes of the mesh , non-modifiable access
  const CNodes& nodes() const;
  
  void signal_write_mesh ( Common::XmlNode& node );

  void signature_write_mesh ( Common::XmlNode& node);

private:
  
  boost::shared_ptr<Common::CLink> m_nodes_link;
  
  boost::shared_ptr<CRegion> m_topology;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_hpp
