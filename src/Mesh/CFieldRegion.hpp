// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFieldRegion_hpp
#define CF_Mesh_CFieldRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"


namespace CF {

namespace Common 
{
  class CLink;
}

namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to fields (CFieldRegion)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CFieldRegion : public CRegion {

public: // typedefs

  typedef boost::shared_ptr<CFieldRegion> Ptr;
  typedef boost::shared_ptr<CFieldRegion const> ConstPtr;
  
public: // functions

  /// Contructor
  /// @param name of the component
  CFieldRegion ( const std::string& name );

  /// Virtual destructor
  virtual ~CFieldRegion();

  /// Get the class name
  static std::string type_name () { return "CFieldRegion"; }

  // functions specific to the CFieldRegion component
  
  /// create a CFieldRegion component
  /// @param name of the field
  CFieldRegion& synchronize_with_region(CRegion& support, const std::string& field_name = "");

  /// create a CElements component, initialized to take connectivity data for the given type
  /// @param name of the field
  /// @param element_type_name type of the elements
  CElements& create_elements (CElements& geometry_elements);
    
  const CRegion& support() const;
  CRegion& support();
    
  std::string field_name() const { return m_tree_name; }
  
private:
  
  std::string m_tree_name;

  boost::shared_ptr<Common::CLink> m_support;
  
};

////////////////////////////////////////////////////////////////////////////////

  
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFieldRegion_hpp
